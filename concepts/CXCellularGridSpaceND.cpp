#pragma once
#include <concepts>
#include <type_traits>
#include <boost/concept/assert.hpp>
#include <DGtal/base/Assert.h>
#include <DGtal/topology/CCellularGridSpaceND.h>
#include <DGtal/topology/CPreCellularGridSpaceND.h>
#include "BaseConcepts.cpp"
#include "../util/DimensionOf.cpp"
#include "CCell.cpp"

namespace DGtalTrace {
namespace concepts {

namespace bridges {
namespace detail {

template <typename TKSpace, DGtal::Dimension dim = 0>
concept CKXSpaceLite = requires {
  typename TKSpace::PreCellularGridSpace;
  typename TKSpace::Point;
  requires CPoint<typename TKSpace::Point>;
  typename TKSpace::Cell;
  typename TKSpace::SCell;
  typename TKSpace::Cells;
  requires std::same_as<typename TKSpace::Cell, typename TKSpace::Cells::value_type>;
  requires(dim == 0 || DimensionOf<TKSpace> == dim);
  requires HasSameDimension<TKSpace, typename TKSpace::PreCellularGridSpace>;
  requires HasSameDimension<TKSpace, typename TKSpace::Point>;
  requires HasSameDimension<TKSpace, typename TKSpace::Cell>;
  requires HasSameDimension<TKSpace, typename TKSpace::SCell>;
} && requires(const TKSpace &ks, const TKSpace::Cell &c) {
  { ks.uTopology(c) } -> std::same_as<typename TKSpace::Integer>;
  { ks.uLowerIncident(c) } -> std::same_as<typename TKSpace::Cells>;
  { ks.uFaces(c) } -> std::same_as<typename TKSpace::Cells>;
};

template <typename TKSpace, int dim = 0>
concept CKPreSpaceLite =
    CKXSpaceLite<TKSpace, dim> &&
    std::same_as<TKSpace, typename TKSpace::PreCellularGridSpace>;

template <typename TKSpace, int dim = 0>
concept CKSpaceLite = CKXSpaceLite<TKSpace, dim> && requires {
  requires !CKPreSpaceLite<TKSpace, dim>;
  typename TKSpace::PreCell;
  typename TKSpace::SPreCell;
  requires CUPreCell<typename TKSpace::PreCell, dim>;
  requires CSPreCell<typename TKSpace::SPreCell, dim>;
} && requires(const TKSpace &ks, const TKSpace::Cell &c) {
  { ks.uFirst(c) } -> std::same_as<typename TKSpace::Cell>;
  { ks.uLast(c) } -> std::same_as<typename TKSpace::Cell>;
  { ks.uFaces(c) } -> std::same_as<typename TKSpace::Cells>;
};

} // namespace detail

template <typename T, int dim, typename = void>
struct CCellularGridSpaceND : std::false_type {};

template <typename T, int dim>
  requires detail::CKSpaceLite<T, dim>
struct CCellularGridSpaceND<T, dim> : std::true_type {
  BOOST_CONCEPT_ASSERT((DGtal::concepts::CCellularGridSpaceND<T>));
};

template <typename T, int dim, typename = void>
struct CPreCellularGridSpaceND : std::false_type {};

template <typename T, int dim>
  requires detail::CKPreSpaceLite<T, dim>
struct CPreCellularGridSpaceND<T, dim> : std::true_type {
  BOOST_CONCEPT_ASSERT((DGtal::concepts::CPreCellularGridSpaceND<T>));
};

} // namespace bridges

template <typename T, int dim = 0>
concept CCellularGridSpaceND = requires {
  typename bridges::CCellularGridSpaceND<T, dim>;
} && bridges::CCellularGridSpaceND<T, dim>::value;

template <typename T, int dim = 0>
concept CPreCellularGridSpaceND =
    bridges::CPreCellularGridSpaceND<T, dim>::value &&
    !CCellularGridSpaceND<T, dim>;

template <typename T, int dim = 0>
concept CXCellularGridSpaceND =
    CCellularGridSpaceND<T, dim> || CPreCellularGridSpaceND<T, dim>;

//template<typename T, typename ... U>
//concept CSameAsAny = (std::same_as<T, U> || ...);

template <typename KPreSpace, typename ... TCell>
concept CHasKPreSpace = (CXXCell<TCell> && ...) && 
                        CPreCellularGridSpaceND<KPreSpace> &&                         
                        (std::same_as<KPreSpace, typename TCell::PreCellularGridSpace> && ...);

template <typename KXSpace, typename ... TCell>
concept CHasKSpace = (CXXCell<TCell> && ...) && 
                     CXCellularGridSpaceND<KXSpace> &&
                     (std::same_as<KXSpace, typename TCell::CellularGridSpace> && ...);

/*
template <typename KXSpace, typename ... TCell>
concept CHasCell = CXCellularGridSpaceND<KXSpace> &&
                   (CXXCell<TCell> && ...) && 
                   (CHasKSpace<KSpace, TCell> && ...);

template <typename KXSpace, typename ... TCell>
concept CHasXCell = CXCellularGridSpaceND<KXSpace> &&
                    (CXXCell<TCell> && ...) && 
                    ((CHasKSpace<KSpace, TCell> || CHasKPreSpace<KSpace, TCell>) && ...);
*/

namespace {
/*
using namespace DGtal::Z2i;

static_assert((CCellularGridSpaceND<KSpace, 2> == true));
static_assert((CCellularGridSpaceND<KSpace, 3> == false));

static_assert((CCellularGridSpaceND<KSpace> == true));
static_assert((CCellularGridSpaceND<KPreSpace> == false));
static_assert((CPreCellularGridSpaceND<KPreSpace> == true));
static_assert((CPreCellularGridSpaceND<KSpace> == false));

static_assert((CHasKSpace<KSpace, KSpace::Cell> == true));
static_assert((CHasKSpace<KSpace, KSpace::SCell> == true));
static_assert((CHasKSpace<KSpace, KSpace::PreCell> == false));
static_assert((CHasKSpace<KSpace, KSpace::SPreCell> == false));
static_assert((CHasKSpace<KPreSpace, KPreSpace::Cell> == true));
static_assert((CHasKSpace<KPreSpace, KPreSpace::SCell> == true));
static_assert((CHasKSpace<DGtal::Z3i::KSpace::Cell, KSpace> == false));

static_assert((CHasKPreSpace<KPreSpace, KSpace::Cell> == true));
static_assert((CHasKPreSpace<KPreSpace, KSpace::SCell> == true));
static_assert((CHasKPreSpace<KPreSpace, KSpace::PreCell> == true));
static_assert((CHasKPreSpace<KPreSpace, KSpace::SPreCell> == true));
static_assert((CHasKPreSpace<KPreSpace, KPreSpace::Cell> == true));
static_assert((CHasKPreSpace<KPreSpace, KPreSpace::SCell> == true));
static_assert((CHasKPreSpace<KPreSpace, DGtal::Z3i::KSpace::Cell> == false));
*/
} // anonymous namespace

} // namespace concepts
} // namespace DGtalTrace