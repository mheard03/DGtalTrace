#pragma once
#include <DGtal/helpers/StdDefs.h>
#include <DGtal/base/Common.h>
#include <DGtal/kernel/domains/HyperRectDomain.h>
#include <DGtal/topology/CCellularGridSpaceND.h>
#include <DGtal/kernel/domains/Linearizer.h>


template <typename TKSpace, typename TStorageOrder = DGtal::ColMajorStorage>
struct KSpaceLinearizer {
  BOOST_CONCEPT_ASSERT(( DGtal::concepts::CCellularGridSpaceND<TKSpace> ));
  typedef TKSpace KSpace;
  typedef DGtal::Linearizer<DGtal::HyperRectDomain<typename KSpace::Space>, TStorageOrder> Linearizer;
  typedef typename Linearizer::Domain Domain;
  typedef typename Linearizer::Point Point;
  typedef typename Linearizer::Extent Extent;
  typedef typename Linearizer::Size Size;

 private:
  static inline Extent getExtent(const KSpace& K) {
    return K.uKCoords(K.upperCell()) - K.uKCoords(K.lowerCell()) + Point::diagonal(1);
  }

 public:
  static inline Size getIndex(const Point& point, const KSpace& K) {
    return Linearizer::getIndex(point, K.uKCoords(K.lowerCell()), getExtent(K));
  }

  static inline Point getPoint(const Size index, const KSpace& K) {
    return Linearizer::getPoint(index, getExtent(K));
  }

};