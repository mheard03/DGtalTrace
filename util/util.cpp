#pragma once
#include <DGtal/helpers/StdDefs.h>
#include <DGtal/kernel/PointVector.h>
#include "DGtal/base/Common.h"


/*
template <typename TPoint, typename Comparator> requires DGtal::IsAPointVector<TPoint>::value
constexpr bool pointCompare(const TPoint& p1, const TPoint& p2, Comparator compare = std::equal_to<TPoint>()) {
  for (DGtal::Dimension k = 0; k < TPoint::dimension; ++k) {
    if (!compare(p1[k], p2[k])) return false;
  }
  return true;
}
*/