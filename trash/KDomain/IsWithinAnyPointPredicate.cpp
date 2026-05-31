#pragma once
#include <boost/concept/assert.hpp>
#include <DGtal/base/Assert.h>
#include <DGtal/base/Common.h>
#include <DGtal/helpers/StdDefs.h>
#include <DGtal/kernel/CPointPredicate.h>

namespace DGtal {
namespace functors {

template <typename TPoint>
struct IsWithinAnyPointPredicate {
  typedef TPoint Point;
  typedef DGtal::functors::IsWithinPointPredicate<Point> SubPredicate;

  IsWithinAnyPointPredicate() {}

  void add(const SubPredicate& subPredicate) {
    mySubPredicates.push_back(subPredicate);
  }

  bool operator()(const Point & p ) const {
    for (const SubPredicate &subPred : mySubPredicates) {
      if (subPred(p)) return true;
    }
    return false;
  }

 private:
  std::vector<SubPredicate> mySubPredicates;
};
BOOST_CONCEPT_ASSERT (( DGtal::concepts::CPointPredicate<IsWithinAnyPointPredicate<DGtal::Z2i::Point>> ));

} // namespace functors
} // namespace DGtal