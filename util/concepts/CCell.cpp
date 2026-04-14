#pragma once
#include <DGtal/base/Assert.h>
#include <boost/iostreams/filter/symmetric.hpp>
#include <concepts>
#include <type_traits>
#include <DGtal/base/Common.h>
#include <DGtal/helpers/StdDefs.h>
#include "../DimensionOf.cpp"

namespace DGtalTrace {
namespace concepts {
namespace detail {
  template <typename TCell, int dim = -1>
  concept CXXCellBase = requires {
    typename TCell::Integer;
    typename TCell::UnsignedInteger;
    typename TCell::Point;
    typename TCell::PreCellularGridSpace;
    typename TCell::CellularGridSpace;
    HasSameDimension<TCell, typename TCell::Point>;
    dim == -1 || DimensionOf<TCell> == dim;
  } && requires (TCell c) {
    c.preCell();
    c.preCell().coordinates;
  };

  template <typename T, typename U>
  concept IsBasically = std::same_as<std::remove_cvref_t<T>, std::remove_cvref_t<U>>;

  template <typename TCell, int is_signed = -1>
  concept CCellHasSignedness = requires {
    requires is_signed == -1;
  } || requires (TCell c) {
    requires is_signed == false;
    { c.preCell() } -> IsBasically<typename TCell::PreCell>;
  } || requires (TCell c) {
    requires is_signed == true;
    { c.preCell() } -> IsBasically<typename TCell::SPreCell>;
  };

  template <typename TCell>
  concept CCellIsPreCell = requires (TCell c) {
    std::same_as<typename TCell::CellularGridSpace, typename TCell::PreCellularGridSpace>;
    { c.preCell() } -> IsBasically<TCell>;
  };

  template <typename TCell, int is_precell = -1>
  concept CCellHasPrecellness = requires {
    requires is_precell == -1;
  } || requires (TCell c) {
    requires is_precell == CCellIsPreCell<TCell>;
  };  

  template <typename TCell, int dim = -1, int is_signed = -1, int is_precell = -1>
  concept CCellProps = CXXCellBase<TCell, dim> 
       && CCellHasSignedness<TCell, is_signed>
       && CCellHasPrecellness<TCell, is_precell>;
 
} // namespace detail

template <typename TCell, int dim = -1> concept CXXCell =   detail::CCellProps<TCell, dim, -1, -1>;
template <typename TCell, int dim = -1> concept CXCell =    detail::CCellProps<TCell, dim, -1,  0>;
template <typename TCell, int dim = -1> concept CXPreCell = detail::CCellProps<TCell, dim, -1,  1>;
template <typename TCell, int dim = -1> concept CUXCell =   detail::CCellProps<TCell, dim,  0, -1>;
template <typename TCell, int dim = -1> concept CUCell =    detail::CCellProps<TCell, dim,  0,  0>;
template <typename TCell, int dim = -1> concept CUPreCell = detail::CCellProps<TCell, dim,  0,  1>;
template <typename TCell, int dim = -1> concept CSXCell =   detail::CCellProps<TCell, dim,  1, -1>;
template <typename TCell, int dim = -1> concept CSCell =    detail::CCellProps<TCell, dim,  1,  0>;
template <typename TCell, int dim = -1> concept CSPreCell = detail::CCellProps<TCell, dim,  1,  1>;

template <typename TCell, int dim = -1> concept CCell = CUCell<TCell, dim>;
template <typename TCell, int dim = -1> concept CPreCell = CUPreCell<TCell, dim>;

namespace {
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
} // anonymous namespace

} // namespace concepts
} // namespace DGtalTrace