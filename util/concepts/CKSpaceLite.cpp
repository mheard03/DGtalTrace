#pragma once
#include <concepts>
#include <DGtal/base/Assert.h>
#include <DGtal/base/Common.h>
#include <DGtal/helpers/StdDefs.h>
#include "BaseConcepts.cpp"
#include "../DimensionOf.cpp"
#include "CCell.cpp"

namespace DGtalTrace {
namespace concepts {

template <typename TKSpace, DGtal::Dimension dim = 0>
concept CKXSpaceLite = requires {
  typename TKSpace::PreCellularGridSpace;
  typename TKSpace::Point;
  requires CPoint<typename TKSpace::Point>;
  typename TKSpace::Cell;
  typename TKSpace::SCell;
  typename TKSpace::Cells;
  std::same_as<typename TKSpace::Cell, typename KSpace::Cells::value_type>;
  requires (dim == 0 || DimensionOf<TKSpace> == dim);
  requires HasSameDimension<TKSpace, typename TKSpace::PreCellularGridSpace>;
  requires HasSameDimension<TKSpace, typename TKSpace::Point>;
  requires HasSameDimension<TKSpace, typename TKSpace::Cell>;
  requires HasSameDimension<TKSpace, typename TKSpace::SCell>;
} && requires (const TKSpace& ks, const TKSpace::Cell& c) {
  { ks.uTopology(c) }      -> std::same_as<typename TKSpace::Integer>;  
  { ks.uLowerIncident(c) } -> std::same_as<typename TKSpace::Cells>;  
  { ks.uFaces(c) }         -> std::same_as<typename TKSpace::Cells>;
};

template <typename TKSpace, int dim = 0>
concept CKPreSpaceLite = CKXSpaceLite<TKSpace, dim> && 
  std::same_as<TKSpace, typename TKSpace::PreCellularGridSpace>;

template <typename TKSpace, int dim = 0>
concept CKSpaceLite = CKXSpaceLite<TKSpace, dim> && requires {
  requires !CKPreSpaceLite<TKSpace, dim>;
  typename TKSpace::PreCell;
  typename TKSpace::SPreCell;
  requires CUPreCell<typename TKSpace::PreCell, dim>;
  requires CSPreCell<typename TKSpace::SPreCell, dim>;
} && requires (const TKSpace& ks, const TKSpace::Cell& c) {
  { ks.uFirst(c) } -> std::same_as<typename TKSpace::Cell>;
  { ks.uLast(c) }  -> std::same_as<typename TKSpace::Cell>;  
  { ks.uFaces(c) } -> std::same_as<typename TKSpace::Cells>;
};

namespace {
  using namespace DGtal::Z2i;

  static_assert((CKXSpaceLite<KSpace> == true));
  static_assert((CKXSpaceLite<KSpace, 2> == true));
  static_assert((CKXSpaceLite<KSpace, 3> == false));
  static_assert((CKXSpaceLite<KPreSpace> == true));
  static_assert((CKXSpaceLite<KPreSpace, 2> == true));
  static_assert((CKXSpaceLite<KPreSpace, 3> == false));

  static_assert((CKSpaceLite<KSpace> == true));
  static_assert((CKSpaceLite<KPreSpace> == false));

  static_assert((CKPreSpaceLite<KPreSpace> == true));
  static_assert((CKPreSpaceLite<KSpace> == false));  
} // anonymous namespace

} // namespace concepts
} // namespace DGtalTrace