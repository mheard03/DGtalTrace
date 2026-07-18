#pragma once
#include <concepts>
#include <boost/concept_check.hpp>
#include <boost/concept/assert.hpp>
#include <boost/concept/requires.hpp>
#include <DGtal/base/CConstBidirectionalRangeFromPoint.h>
#include "../../BaseConcepts.cpp"
#include "CConstBidirectionalRange.cpp"


namespace DGtalTrace {
namespace concepts {

namespace bridges {
namespace detail {

#define ISBASICALLY DGtalTrace::concepts::detail::IsBasically

template <typename T>
concept CConstBidirectionalRangeFromPointImpl = CConstBidirectionalRangeImpl<T> && 
requires {
  typename T::Point;
  typename T::ConstReverseIterator;
} && 
requires (T range, typename T::Point point) {
  { range.rbegin(point) } -> ISBASICALLY<typename T::ConstReverseIterator>;
};

#undef ISBASICALLY

} // namespace detail

template <typename T, typename = void>
struct CConstBidirectionalRangeFromPoint : std::false_type {};

template <typename T>
requires detail::CConstBidirectionalRangeFromPointImpl<T>
struct CConstBidirectionalRangeFromPoint<T> : std::true_type {
  BOOST_CONCEPT_ASSERT((DGtal::concepts::CConstBidirectionalRangeFromPoint<T>));
};

} // namespace bridges

template <typename T>
concept CConstBidirectionalRangeFromPoint = bridges::detail::CConstBidirectionalRangeFromPointImpl<T> && 
  CIsTrue<bridges::CConstBidirectionalRangeFromPoint<T>>;

} // namespace concepts
} // namespace DGtalTrace