#pragma once
#include <concepts>
#include <boost/concept_check.hpp>
#include <boost/concept/assert.hpp>
#include <boost/concept/requires.hpp>
#include <DGtal/base/CConstSinglePassRange.h>
#include "../../BaseConcepts.cpp"

namespace DGtalTrace {
namespace concepts {

namespace bridges {
namespace detail {

#define ISBASICALLY DGtalTrace::concepts::detail::IsBasically

template <typename T>
concept CConstSinglePassRangeImpl = requires (T range) {
  typename T::ConstIterator;
  requires std::input_iterator<typename T::ConstIterator>;
  { range.begin() } -> ISBASICALLY<typename T::ConstIterator>;
  { range.end() } -> ISBASICALLY<typename T::ConstIterator>;
};

#undef ISBASICALLY

} // namespace detail

template <typename T, typename = void>
struct CConstSinglePassRange : std::false_type {};

template <typename T>
requires detail::CConstSinglePassRangeImpl<T>
struct CConstSinglePassRange<T> : std::true_type {
  BOOST_CONCEPT_ASSERT((DGtal::concepts::CConstSinglePassRange<T>));
};

} // namespace bridges

template <typename T>
concept CConstSinglePassRange = bridges::detail::CConstSinglePassRangeImpl<T> && 
  CIsTrue<bridges::CConstSinglePassRange<T>>;

} // namespace concepts
} // namespace DGtalTrace