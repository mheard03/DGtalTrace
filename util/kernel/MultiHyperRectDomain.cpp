#pragma once
#include <DGtal/helpers/StdDefs.h>
#include <DGtal/kernel/domains/CDomain.h>
#include <iostream>
#include "DGtal/base/Common.h"
#include "DGtal/base/BasicBoolFunctors.h"
#include "DGtal/base/CPredicate.h"
#include "DGtal/base/ConstAlias.h"
#include "DGtal/kernel/CPointFunctor.h"
#include "DGtal/kernel/CPointPredicate.h"
#include "DGtal/kernel/BasicPointPredicates.h"
#include <boost/concept/assert.hpp>

namespace DGtal {
namespace functors {

typedef DGtal::Z2i::Point TPoint;
//  template <typename TPoint>
struct IsWithinAnyPointPredicate {
  typedef TPoint Point;
  typedef IsWithinPointPredicate<Point> SubPredicate;

  IsWithinAnyPointPredicate() {}

  void addSubPredicate(const SubPredicate& subPredicate) {
    mySubPredicates.push_back(subPredicate);
  }

  bool operator()(const Point & p ) const {
    for (const SubPredicate &subPred : mySubPredicates) {
      if (subPred(p)) return true;
    }
    return false;
  }

  std::vector<SubPredicate> mySubPredicates;
};
BOOST_CONCEPT_ASSERT (( concepts::CPointPredicate<IsWithinAnyPointPredicate> ));

} } // end namespace DGtal::functors

namespace DGtal {

  typedef DGtal::Z2i::Domain TDomain;
  template  //  template <typename TDomain, bool isSingleDomain = false>
  class MultiHyperRectDomain {

    std::vector<TDomain> myDomains;
    BOOST_CONCEPT_ASSERT(( concepts::CDomain<TDomain> ));
  };
}