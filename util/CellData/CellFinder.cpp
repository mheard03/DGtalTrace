#pragma once
#include <DGtal/base/BasicTypes.h>
#include <DGtal/base/Common.h>
#include <DGtal/helpers/StdDefs.h>
#include <DGtal/kernel/PointVector.h>
#include <DGtal/kernel/SpaceND.h>
#include <DGtal/kernel/domains/HyperRectDomain.h>
#include <DGtal/topology/KhalimskySpaceND.h>
#include <DGtal/base/CSTLAssociativeContainer.h>
#include <concepts>
#include <type_traits>
#include "Concepts.cpp"
#include <DGtal/kernel/domains/Linearizer.h>

namespace DGtalTrace {
namespace CellData {
  
template <concepts::CCell TCell>
struct CellLinearizer {
  typedef TCell::PreCellularGridSpace KPreSpace;
  typedef DGtal::HyperRectDomain<typename KPreSpace::Space> Domain;
  typedef Domain::Point Point;
  typedef Domain::Point Extent;
  typedef Space::Size Size;
  typedef DGtal::Linearizer<Domain> PointLinearizer;

  static inline Size getIndex(Point aPoint, Point const& aLowerBound, Extent const& anExtent) {
    return PointLinearizer::getIndex(std::forward(aPoint, aLowerBound, anExtent));
  }

  static inline Size getIndex(Point aPoint, Extent const& anExtent) {
    return PointLinearizer::getIndex(std::forward(aPoint, aLowerBound, anExtent));
  }

  /** Linearized index of a point, given a domain.
    *
    * @param[in] aPoint    The Point to be linearized.
    * @param[in] aDomain   The domain.
    * @return the linearized index of the point.
    */
  static inline
  Size getIndex( Point aPoint, Domain const& aDomain );

  /** De-linearization of an index, given the domain lower-bound and extent.
    *
    * @param[in] anIndex     The linearized index.
    * @param[in] aLowerBound The lower-bound of the domain.
    * @param[in] anExtent    The domain extent.
    * @return  the point whose linearized index is anIndex.
    */
  static inline
  Point getPoint( Size anIndex, Point const& aLowerBound, Extent const& anExtent );

  /** De-linearization of an index, given the domain extent.
    *
    * The lower-bound of the domain is set to the origin.
    *
    * @param[in] anIndex   The linearized index.
    * @param[in] anExtent  The domain extent.
    * @return  the point whose linearized index is anIndex.
    */
  static inline
  Point getPoint( Size anIndex, Extent const& anExtent );

  /** De-linearization of an index, given a domain.
    *
    * @param[in] anIndex   The linearized index.
    * @param[in] aDomain   The domain.
    * @return  the point whose linearized index is anIndex.
    */
  static inline
  Point getPoint( Size anIndex, Domain const& aDomain );
};

  /**
   * @brief Aim: Linearization and de-linearization interface for HyperRectDomain.
   *
   * This is a static class that provides point linearization (point to index) and de-linearization (index to point) for storages working on HyperRectDomain.
   *
   * The storage order can be specified by template (default is column-major ordered).
   *
   * Example:
   * @code
   * typedef SpaceND<2>             Space;
   * tydedef HyperRectDomain<Space> Domain;
   * typedef typename Space::Point  Point;
   *
   * const Domain domain( Point(0, 1), Point(4, 3) );
   *
   * typename Linearizer<Domain>::Size id = Linearizer<Domain>::getIndex( Point(2, 2), domain ); // returns 7.
   *
   * Point pt = Linearizer<Domain>::getPoint( 7, domain); // returns Point(2,2).
   * @endcode
   *
   * @tparam  TSpace       Type of the space of the HyperRectDomain (auto-deduced from TDomain template, see Linearizer).
   * @tparam  TStorageOrder Storage Order (RowMajorStorage of ColMajorStorage).
   */
  template <
      typename TSpace,
      typename TStorageOrder
    >
  struct Linearizer< HyperRectDomain<TSpace>, TStorageOrder >
    {
      // Useful typedefs
      typedef HyperRectDomain<TSpace> Domain; ///< The domain type.
      typedef typename TSpace::Point Point;   ///< The point type.
      typedef Point Extent;                   ///< The domain's extent type.
      typedef typename TSpace::Size  Size;    ///< The space's size type.

      /** Linearized index of a point, given the domain lower-bound and extent.
       *
       * @param[in] aPoint      The point to be linearized.
       * @param[in] aLowerBound The lower-bound of the domain.
       * @param[in] anExtent    The extent of the domain.
       * @return the linearized index of the point.
       */
      static inline
      Size getIndex( Point aPoint, Point const& aLowerBound, Extent const& anExtent );

      /** Linearized index of a point, given the domain extent.
       *
       * The lower-bound of the domain is defined to the origin.
       *
       * @param[in] aPoint    The Point to be linearized.
       * @param[in] anExtent  The extent of the domain.
       * @return the linearized index of the point.
       */
      static inline
      Size getIndex( Point aPoint, Extent const& anExtent );

      /** Linearized index of a point, given a domain.
       *
       * @param[in] aPoint    The Point to be linearized.
       * @param[in] aDomain   The domain.
       * @return the linearized index of the point.
       */
      static inline
      Size getIndex( Point aPoint, Domain const& aDomain );

      /** De-linearization of an index, given the domain lower-bound and extent.
       *
       * @param[in] anIndex     The linearized index.
       * @param[in] aLowerBound The lower-bound of the domain.
       * @param[in] anExtent    The domain extent.
       * @return  the point whose linearized index is anIndex.
       */
      static inline
      Point getPoint( Size anIndex, Point const& aLowerBound, Extent const& anExtent );

      /** De-linearization of an index, given the domain extent.
       *
       * The lower-bound of the domain is set to the origin.
       *
       * @param[in] anIndex   The linearized index.
       * @param[in] anExtent  The domain extent.
       * @return  the point whose linearized index is anIndex.
       */
      static inline
      Point getPoint( Size anIndex, Extent const& anExtent );

      /** De-linearization of an index, given a domain.
       *
       * @param[in] anIndex   The linearized index.
       * @param[in] aDomain   The domain.
       * @return  the point whose linearized index is anIndex.
       */
      static inline
      Point getPoint( Size anIndex, Domain const& aDomain );

  }; // end of class Linearizer

} // namespace DGtal















namespace cellData {
namespace detail {
template <concepts::CCell TCell>
class PointToCellFunctor {
 public:
  typedef typename TCell::PreCellularGridSpace PKSpace;
  typedef typename TCell::Point Point;
  TCell operator() (const Point& p) {
    TCell result(0);
    PKSpace::uSetKCoords(result, p);
    return result;
  }
};
} // namespace detail;

template <concepts::CCell TCell>
class IntToCellFunctor {
 public:
 DGtal
  typedef  DGtal::SpaceND<DGtalTrace::DimensionOf<TPoint>, uint>,  Size;
  typedef typename TCell::PreCellularGridSpace PKSpace;
  typedef typename TCell::Point Point;
  TCell operator() (const Point& p) {
    TCell result(0);
    PKSpace::uSetKCoords(result, p);
    return result;
  }
};

typedef DGtal::Z2i::KSpace::Space Space;
typedef DGtal::Z2i::Space::Size Size;
typedef DGtal::Z2i::Space::Point::Component  Size2;

/*
template <concepts::CCell TCell>
class PointToCellFunctor {
 public:
  typedef typename TCell::CellularGridSpace KSpace;
  typedef typename TCell::PreCellularGridSpace PKSpace;
  PointToCellFunctor() : myKPtr(nullptr) {};
  PointToCellFunctor(DGtal::ConstAlias<KSpace> ks) : myKPtr(&ks) {};

  typedef typename TCell::Point Point;
  TCell operator() (const Point& p) const {
    TCell result(0);
    if (myKPtr == nullptr) {
      PKSpace::uSetKCoords(result, p);
    }
    else {
      myKPtr->uSetKCoords(result, p);
    }
    return result;
  }
 private:
  const PointToPreCellFunctor<TCell> pcf{};
  const KSpace *myKPtr;
};


typedef DGtal::Z2i::KPreSpace PK;
typedef DGtal::Z2i::KSpace::Cell C;
typedef DGtal::Z2i::KSpace::PreCell PC;
typedef PC::Point PCP;
typedef PointToPreCellFunctor<PC> pcf;

PCP p({3, 3});
PC cell = pcf{}(p);
PC cell2 = PK::uCell(p);
C c(0);


*/


/*
typedef typename DGtal::Z2i::KSpace KSpace;
typedef typename KSpace::Cell Cell;
typedef typename KSpace::PreCell PreCell;
const KSpace k({0, 0}, {12, 12}, true);
const Cell c = k.uCell({0, 0});
const PreCell pc(c);  // ok
const Cell c2(pc);    // illegal because private
const PreCell pc2({3, 3});
*/




} // namespace CellData
} // namespace DGtalTrace