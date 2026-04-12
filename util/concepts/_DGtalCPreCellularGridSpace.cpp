#pragma once
#include <DGtal/base/Common.h>
#include <DGtal/helpers/StdDefs.h>
#include <DGtal/kernel/PointVector.h>
#include "CCell.cpp"

namespace DGtalTrace {
namespace concepts {

namespace detail {
// ============================================================
// Helper: checks that a template rebinder T::XxxMap<V>::Type exists
// ============================================================

template <typename T>
concept HasCellMaps = requires {
  typename T::template CellMap<int>::Type;
  typename T::template SCellMap<int>::Type;
  typename T::template SurfelMap<int>::Type;
};

// CConstSinglePassRange: needs begin()/end() returning an iterator
template <typename R>
concept CConstSinglePassRange = requires(const R &r) {
  { r.begin() } -> std::input_iterator;
  { r.end() };
};

// UniqueAssociativeContainer (set-like): find, insert, no duplicate keys
template <typename C>
concept UniqueAssociativeContainer = requires(C c, typename C::key_type k) {
  { c.find(k) } -> std::same_as<typename C::iterator>;
  { c.insert(k) };
  { c.size() } -> std::convertible_to<std::size_t>;
};

// PairAssociativeContainer (map-like): mapped_type exists, operator[] works
template <typename C>
concept PairAssociativeContainer =
    UniqueAssociativeContainer<C> && requires(C c, typename C::key_type k) {
      typename C::mapped_type;
      { c[k] } -> std::convertible_to<typename C::mapped_type &>;
    };

// ============================================================
// Layer 1: associated types exist
// ============================================================
template <typename T>
concept CPreCellularGridSpaceND_Types = requires {
  // static constants/types
  typename T::Integer;
  typename T::Space;
  typename T::PreCellularGridSpace;
  typename T::Cell;
  typename T::SCell;
  typename T::Surfel;
  typename T::Sign;
  typename T::DirIterator;
  typename T::Point;
  typename T::Vector;
  typename T::Cells;
  typename T::SCells;
  typename T::CellSet;
  typename T::SCellSet;
  typename T::SurfelSet;
} && detail::HasCellMaps<T>;

// ============================================================
// Layer 2: static members and type consistency
// ============================================================
template <typename T>
concept CPreCellularGridSpaceND_Static =
    CPreCellularGridSpaceND_Types<T> &&
    requires {
      { T::dimension } -> std::convertible_to<DGtal::Dimension>;
      { T::DIM } -> std::convertible_to<DGtal::Dimension>;
      { T::POS } -> std::convertible_to<typename T::Sign>;
      { T::NEG } -> std::convertible_to<typename T::Sign>;
    } &&
    // Integer must match Space::Integer, Point and Vector likewise
    std::convertible_to<typename T::Integer, typename T::Space::Integer> &&
    std::convertible_to<typename T::Point, typename T::Space::Point> &&
    std::convertible_to<typename T::Vector, typename T::Space::Vector>;

// ============================================================
// Layer 3: container concept constraints on associated types
// ============================================================
template <typename T>
concept CPreCellularGridSpaceND_Containers =
    CPreCellularGridSpaceND_Static<T> &&
    detail::CConstSinglePassRange<typename T::Cells> &&
    detail::CConstSinglePassRange<typename T::SCells> &&
    detail::UniqueAssociativeContainer<typename T::CellSet> &&
    detail::UniqueAssociativeContainer<typename T::SCellSet> &&
    detail::UniqueAssociativeContainer<typename T::SurfelSet> &&
    detail::PairAssociativeContainer<typename T::template CellMap<int>::Type> &&
    detail::PairAssociativeContainer<
        typename T::template SCellMap<int>::Type> &&
    detail::PairAssociativeContainer<typename T::template SurfelMap<int>::Type>;

// ============================================================
// Layer 4: const method expressions (the bulk)
// ============================================================
template <typename T>
concept CPreCellularGridSpaceND_ConstMethods =
    CPreCellularGridSpaceND_Containers<T> &&
    requires(const T x, typename T::Cell c, typename T::SCell sc,
             typename T::Point p, typename T::Vector v, typename T::Integer i,
             typename T::Sign sign, DGtal::Dimension k, bool b) {
      // ---- cell construction ----
      { x.uCell(p) } -> std::same_as<typename T::Cell>;
      { x.uCell(p, c) } -> std::same_as<typename T::Cell>;
      { x.sCell(p) } -> std::same_as<typename T::SCell>;
      { x.sCell(p, sign) } -> std::same_as<typename T::SCell>;
      { x.sCell(p, sc) } -> std::same_as<typename T::SCell>;
      { x.uSpel(p) } -> std::same_as<typename T::Cell>;
      { x.sSpel(p) } -> std::same_as<typename T::SCell>;
      { x.sSpel(p, b) } -> std::same_as<typename T::SCell>;
      { x.uPointel(p) } -> std::same_as<typename T::Cell>;
      { x.sPointel(p) } -> std::same_as<typename T::SCell>;
      { x.sPointel(p, b) } -> std::same_as<typename T::SCell>;

      // ---- coordinate read access ----
      { x.uKCoord(c, k) } -> std::convertible_to<typename T::Integer>;
      { x.uCoord(c, k) } -> std::convertible_to<typename T::Integer>;
      { x.uKCoords(c) } -> std::convertible_to<typename T::Point>;
      { x.uCoords(c) } -> std::convertible_to<typename T::Point>;
      { x.sKCoord(sc, k) } -> std::convertible_to<typename T::Integer>;
      { x.sCoord(sc, k) } -> std::convertible_to<typename T::Integer>;
      { x.sKCoords(sc) } -> std::convertible_to<typename T::Point>;
      { x.sCoords(sc) } -> std::convertible_to<typename T::Point>;

      // ---- sign / topology / dimension ----
      { x.sSign(sc) } -> std::convertible_to<typename T::Sign>;
      { x.signs(c, sign) } -> std::convertible_to<typename T::SCell>;
      { x.sOpp(sc) } -> std::convertible_to<typename T::SCell>;
      { x.unsigns(sc) } -> std::convertible_to<typename T::Cell>;
      { x.uTopology(c) } -> std::convertible_to<typename T::Integer>;
      { x.sTopology(sc) } -> std::convertible_to<typename T::Integer>;
      { x.uDim(c) } -> std::convertible_to<DGtal::Dimension>;
      { x.sDim(sc) } -> std::convertible_to<DGtal::Dimension>;
      { x.uIsSurfel(c) } -> std::same_as<bool>;
      { x.sIsSurfel(sc) } -> std::same_as<bool>;
      { x.uIsOpen(c, k) } -> std::same_as<bool>;
      { x.sIsOpen(sc, k) } -> std::same_as<bool>;

      // ---- direction iterators ----
      { x.uDirs(c) } -> std::same_as<typename T::DirIterator>;
      { x.sDirs(sc) } -> std::same_as<typename T::DirIterator>;
      { x.uOrthDirs(c) } -> std::same_as<typename T::DirIterator>;
      { x.sOrthDirs(sc) } -> std::same_as<typename T::DirIterator>;
      { x.uOrthDir(c) } -> std::convertible_to<DGtal::Dimension>;
      { x.sOrthDir(sc) } -> std::convertible_to<DGtal::Dimension>;

      // ---- unsigned cell geometry ----
      { x.uGetIncr(c, k) } -> std::convertible_to<typename T::Cell>;
      { x.uGetDecr(c, k) } -> std::convertible_to<typename T::Cell>;
      { x.uIsMax(c, k) } -> std::same_as<bool>;
      { x.uIsMin(c, k) } -> std::same_as<bool>;
      { x.uIsInside(c, k) } -> std::same_as<bool>;
      { x.uGetAdd(c, k, i) } -> std::same_as<typename T::Cell>;
      { x.uGetSub(c, k, i) } -> std::same_as<typename T::Cell>;
      { x.uTranslation(c, v) } -> std::same_as<typename T::Cell>;
      { x.uProjection(c, c, k) } -> std::same_as<typename T::Cell>;

      // ---- signed cell geometry ----
      { x.sGetIncr(sc, k) } -> std::same_as<typename T::SCell>;
      { x.sGetDecr(sc, k) } -> std::same_as<typename T::SCell>;
      { x.sIsMax(sc, k) } -> std::same_as<bool>;
      { x.sIsMin(sc, k) } -> std::same_as<bool>;
      { x.sIsInside(sc, k) } -> std::same_as<bool>;
      { x.sGetAdd(sc, k, i) } -> std::same_as<typename T::SCell>;
      { x.sGetSub(sc, k, i) } -> std::same_as<typename T::SCell>;
      { x.sTranslation(sc, v) } -> std::same_as<typename T::SCell>;
      { x.sProjection(sc, sc, k) } -> std::same_as<typename T::SCell>;

      // ---- neighborhood ----
      { x.uNeighborhood(c) } -> std::same_as<typename T::Cells>;
      { x.uProperNeighborhood(c) } -> std::same_as<typename T::Cells>;
      { x.sNeighborhood(sc) } -> std::same_as<typename T::SCells>;
      { x.sProperNeighborhood(sc) } -> std::same_as<typename T::SCells>;
      { x.uAdjacent(c, k, b) } -> std::same_as<typename T::Cell>;
      { x.sAdjacent(sc, k, b) } -> std::same_as<typename T::SCell>;

      // ---- incidence ----
      { x.uIncident(c, k, b) } -> std::same_as<typename T::Cell>;
      { x.sIncident(sc, k, b) } -> std::same_as<typename T::SCell>;
      { x.uLowerIncident(c) } -> std::same_as<typename T::Cells>;
      { x.uUpperIncident(c) } -> std::same_as<typename T::Cells>;
      { x.sLowerIncident(sc) } -> std::same_as<typename T::SCells>;
      { x.sUpperIncident(sc) } -> std::same_as<typename T::SCells>;
      { x.uFaces(c) } -> std::same_as<typename T::Cells>;
      { x.uCoFaces(c) } -> std::same_as<typename T::Cells>;
      { x.sDirect(sc, k) } -> std::same_as<bool>;
      { x.sDirectIncident(sc, k) } -> std::same_as<typename T::SCell>;
      { x.sIndirectIncident(sc, k) } -> std::same_as<typename T::SCell>;
      { x.interiorVoxel(sc) } -> std::same_as<typename T::Point>;
      { x.exteriorVoxel(sc) } -> std::same_as<typename T::Point>;
    };

// ============================================================
// Layer 5: mutable method expressions
// ============================================================
template <typename T>
concept CPreCellularGridSpaceND_MutableMethods =
    CPreCellularGridSpaceND_ConstMethods<T> &&
    requires(T x, typename T::Cell c, typename T::SCell sc, typename T::Point p,
             typename T::Integer i, typename T::Sign sign, DGtal::Dimension k) {
      // coordinate write access
      x.uSetKCoord(c, k, i);
      x.uSetCoord(c, k, i);
      x.uSetKCoords(c, p);
      x.uSetCoords(c, p);
      x.sSetKCoord(sc, k, i);
      x.sSetCoord(sc, k, i);
      x.sSetKCoords(sc, p);
      x.sSetCoords(sc, p);
      x.sSetSign(sc, sign);
      // in-place projection and iteration
      x.uProject(c, c, k);
      x.sProject(sc, sc, k);
      { x.uNext(c, c, c) } -> std::same_as<bool>;
      { x.sNext(sc, sc, sc) } -> std::same_as<bool>;
    };

} // namespace detail

// ============================================================
// CPreCellularGridSpaceND — full base concept
// ============================================================
template <typename T>
concept CPreCellularGridSpaceND =
    std::default_initializable<T> && std::copy_constructible<T> &&
    detail::CPreCellularGridSpaceND_MutableMethods<T> && requires {
      CPreCell<typename T::PreCell>;
      CSPreCell<typename T::SPreCell>;
    };
} // namespace concepts
} // namespace DGtalTrace