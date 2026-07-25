#pragma once
#include <concepts>
#include <DGtal/topology/KhalimskySpaceND.h>
#include <DGtal/base/Assert.h>
#include "BaseConcepts.cpp"

namespace DGtalTrace {
namespace concepts {
namespace detail {
  template <typename TCell, DGtal::Dimension dim = 0>
  concept CXXCellBase = requires {
    typename TCell::Integer;
    typename TCell::UnsignedInteger;
    typename TCell::Point;
    typename TCell::PreCellularGridSpace;
    typename TCell::CellularGridSpace;
    requires HasSameDimension<TCell, typename TCell::Point>;
    requires (dim == 0 || DimensionOf<TCell> == dim);
  } && requires (TCell c) {
    c.preCell();
    c.preCell().coordinates;
  };

  template <typename TCell>
  constexpr bool CellIsSigned = [] {
    if constexpr (requires (TCell c) { 
      { c.preCell() } -> IsBasically<typename TCell::PreCellularGridSpace::Cell>;
    }) return false;
    if constexpr (requires (TCell c) {
      { c.preCell() } -> IsBasically<typename TCell::PreCellularGridSpace::SCell>; 
    }) return true;
  }();

  template <typename TCell, int is_signed = -1>
  concept CCellHasSignedness = requires {
    requires is_signed == -1;
  } || requires { 
    requires CellIsSigned<TCell> == (bool)is_signed; 
  };

  template <typename TCell>
  constexpr bool CellIsPreCell = requires (TCell c) { 
      std::same_as<typename TCell::CellularGridSpace, typename TCell::PreCellularGridSpace>;
      { c.preCell() } -> IsBasically<TCell>;
  };

  template <typename TCell, int is_precell = -1>
  concept CCellHasPrecellness = requires {
    requires is_precell == -1;
  } || requires {
    requires CellIsPreCell<TCell> == is_precell;
  };

  template <typename TCell, int dim = 0, int is_signed = -1, int is_precell = -1>
  concept CCellProps = CXXCellBase<TCell, dim> 
       && CCellHasSignedness<TCell, is_signed>
       && CCellHasPrecellness<TCell, is_precell>;
 
} // namespace detail

template <typename TCell, int dim = 0> concept CXXCell =   detail::CCellProps<TCell, dim, -1, -1>;
template <typename TCell, int dim = 0> concept CXCell =    detail::CCellProps<TCell, dim, -1,  0>;
template <typename TCell, int dim = 0> concept CXPreCell = detail::CCellProps<TCell, dim, -1,  1>;
template <typename TCell, int dim = 0> concept CUXCell =   detail::CCellProps<TCell, dim,  0, -1>;
template <typename TCell, int dim = 0> concept CUCell =    detail::CCellProps<TCell, dim,  0,  0>;
template <typename TCell, int dim = 0> concept CUPreCell = detail::CCellProps<TCell, dim,  0,  1>;
template <typename TCell, int dim = 0> concept CSXCell =   detail::CCellProps<TCell, dim,  1, -1>;
template <typename TCell, int dim = 0> concept CSCell =    detail::CCellProps<TCell, dim,  1,  0>;
template <typename TCell, int dim = 0> concept CSPreCell = detail::CCellProps<TCell, dim,  1,  1>;

template <typename TCell, int dim = 0> concept CCell = CUCell<TCell, dim>;
template <typename TCell, int dim = 0> concept CPreCell = CUPreCell<TCell, dim>;

template <typename TCell, typename UCell>
concept CSameCellSignedness = CXXCell<TCell> && CXXCell<UCell, DimensionOf<TCell>> && requires {
  requires detail::CellIsSigned<TCell> == detail::CellIsSigned<UCell>;
};

namespace {
  /*
  using namespace DGtal::Z2i;

  static_assert((CSCell<KSpace::SCell, 2> == true));
  static_assert((CSCell<KSpace::SPreCell, 2> == false));
  static_assert((CSCell<KSpace::Cell, 2> == false));
  static_assert((CSCell<KSpace::PreCell, 2> == false));
  static_assert((CSPreCell<KSpace::SCell, 2> == false));
  static_assert((CSPreCell<KSpace::SPreCell, 2> == true));
  static_assert((CSPreCell<KSpace::Cell, 2> == false));
  static_assert((CSPreCell<KSpace::PreCell, 2> == false));
  static_assert((CSXCell<KSpace::SCell, 2> == true));
  static_assert((CSXCell<KSpace::SPreCell, 2> == true));
  static_assert((CSXCell<KSpace::Cell, 2> == false));
  static_assert((CSXCell<KSpace::PreCell, 2> == false));
  static_assert((CUCell <KSpace::SCell, 2> == false));
  static_assert((CUCell <KSpace::SPreCell, 2> == false));
  static_assert((CUCell <KSpace::Cell, 2> == true));
  static_assert((CUCell <KSpace::PreCell, 2> == false));
  static_assert((CUPreCell<KSpace::SCell, 2> == false));
  static_assert((CUPreCell<KSpace::SPreCell, 2> == false));
  static_assert((CUPreCell<KSpace::Cell, 2> == false));
  static_assert((CUPreCell<KSpace::PreCell, 2> == true));
  static_assert((CUXCell<KSpace::SCell, 2> == false));
  static_assert((CUXCell<KSpace::SPreCell, 2> == false));
  static_assert((CUXCell<KSpace::Cell, 2> == true));
  static_assert((CUXCell<KSpace::PreCell, 2> == true));
  static_assert((CXCell<KSpace::SCell, 2> == true));
  static_assert((CXCell<KSpace::SPreCell, 2> == false));
  static_assert((CXCell<KSpace::Cell, 2> == true));
  static_assert((CXCell<KSpace::PreCell, 2> == false));
  static_assert((CXPreCell<KSpace::SCell, 2> == false));
  static_assert((CXPreCell<KSpace::SPreCell, 2> == true));
  static_assert((CXPreCell<KSpace::Cell, 2> == false));
  static_assert((CXPreCell<KSpace::PreCell, 2> == true));
  static_assert((CXXCell<KSpace::SCell, 2> == true));
  static_assert((CXXCell<KSpace::SPreCell, 2> == true));
  static_assert((CXXCell<KSpace::Cell, 2> == true));
  static_assert((CXXCell<KSpace::PreCell, 2> == true));

  static_assert((CSameCellSignedness<KSpace::PreCell, KSpace::PreCell> == true));
  static_assert((CSameCellSignedness<KSpace::PreCell, KSpace::Cell> == true));
  static_assert((CSameCellSignedness<KSpace::PreCell, KSpace::SPreCell> == false));
  static_assert((CSameCellSignedness<KSpace::PreCell, KSpace::SCell> == false));
  */
} // anonymous namespace

} // namespace concepts
} // namespace DGtalTrace