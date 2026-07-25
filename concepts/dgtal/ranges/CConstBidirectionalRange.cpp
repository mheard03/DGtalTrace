#pragma once
#include <DGtal/base/CConstSinglePassRange.h>
#include <concepts>
#include <boost/concept_check.hpp>
#include <boost/concept/assert.hpp>
#include <boost/concept/requires.hpp>
#include <DGtal/base/CConstBidirectionalRange.h>
#include "../../BaseConcepts.cpp"
#include "CConstSinglePassRange.cpp"

namespace DGtalTrace {
namespace concepts {

namespace bridges {
namespace detail {

#define ISBASICALLY DGtalTrace::concepts::detail::IsBasically

template <typename T>
concept CConstBidirectionalRangeImpl = CConstSinglePassRangeImpl<T> &&
requires (T range) {
  typename T::ConstReverseIterator;
  requires std::input_iterator<typename T::ConstReverseIterator>;
  { range.rbegin() } -> ISBASICALLY<typename T::ConstReverseIterator>;
  { range.rend() } -> ISBASICALLY<typename T::ConstReverseIterator>;
};

#undef ISBASICALLY

} // namespace detail

template <typename T, typename = void>
struct CConstBidirectionalRange : std::false_type {};

template <typename T>
requires detail::CConstBidirectionalRangeImpl<T>
struct CConstBidirectionalRange<T> : std::true_type {
  BOOST_CONCEPT_ASSERT((DGtal::concepts::CConstBidirectionalRange<T>));
};

} // namespace bridges

template <typename T>
concept CConstBidirectionalRange = bridges::detail::CConstBidirectionalRangeImpl<T> && 
  CIsTrue<bridges::CConstBidirectionalRange<T>>;

} // namespace concepts
} // namespace DGtalTrace