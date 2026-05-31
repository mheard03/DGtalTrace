#pragma once
#include <concepts>
#include <boost/concept/assert.hpp>
#include <DGtal/base/Common.h>
#include <DGtal/helpers/StdDefs.h>
#include <DGtal/kernel/CPointPredicate.h>
#include <DGtal/topology/CCellularGridSpaceND.h>
#include "../concepts/CCell.cpp"

namespace DGtal {
namespace functors {

template <typename TKSpace, typename TPointPredicate>
requires std::same_as<typename TKSpace::Point, typename TPointPredicate::Point>
struct PointPredicateWrapperCellPredicate {
  typedef TKSpace KSpace;
  typedef TPointPredicate PointPredicate;
  typedef KSpace::Point Point;
  typedef KSpace::PreCellularGridSpace KPreSpace;
  BOOST_CONCEPT_ASSERT (( DGtal::concepts::CCellularGridSpaceND<KSpace> ));
  BOOST_CONCEPT_ASSERT (( DGtal::concepts::CPointPredicate<TPointPredicate> ));
  
  PointPredicateWrapperCellPredicate() : myKPtr(nullptr) {}
  PointPredicateWrapperCellPredicate(DGtal::ConstAlias<KSpace> ks) : myKPtr(&ks) {}
  PointPredicateWrapperCellPredicate(DGtal::ConstAlias<KSpace> ks, const PointPredicate& predicate) : myKPtr(&ks), myPointPredicate(predicate) {}
  
  PointPredicateWrapperCellPredicate(const PointPredicateWrapperCellPredicate& other) = default;
  PointPredicateWrapperCellPredicate & operator=(PointPredicateWrapperCellPredicate const& other) = default;

  template <DGtalTrace::concepts::CXXCell TCell>
  bool operator()(const TCell& c) const {
    return myPointPredicate(c.preCell().coordinates);
  }
  bool operator()(const Point& p) const {
    if (myKPtr->cIsValid(p)) return myPointPredicate(p);
    if (!myKPtr->cIsInside(p)) return false;
    return myPointPredicate(KPreSpace::uCell(p).coordinates);
  }

  PointPredicate& predicate() {
    return myPointPredicate;
  }

 private:
  const KSpace* myKPtr;
  PointPredicate myPointPredicate;
};
BOOST_CONCEPT_ASSERT (( DGtal::concepts::CPointPredicate<PointPredicateWrapperCellPredicate<DGtal::Z2i::KSpace, DGtal::functors::TruePointPredicate<DGtal::Z2i::Point>>> ));

} // namespace functors
} // namespace DGtal