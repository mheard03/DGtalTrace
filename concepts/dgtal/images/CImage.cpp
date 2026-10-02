#pragma once
#include <DGtal/base/CConstBidirectionalRangeFromPoint.h>
#include <DGtal/images/CImage.h>
#include <concepts>
#include <boost/concept_check.hpp>
#include <boost/concept/assert.hpp>
#include <boost/concept/requires.hpp>
#include "../../BaseConcepts.cpp"
#include "../ranges/CConstBidirectionalRangeFromPoint.cpp"
#include "CConstImage.cpp"
#include "CTrivialImage.cpp"

namespace DGtalTrace {
namespace concepts {

namespace bridges {
namespace detail {

#define ISBASICALLY DGtalTrace::concepts::detail::IsBasically

template <typename T>
concept CImageImpl = CConstImageImpl<T> && CTrivialImageImpl<T> &&
requires {
  typename T::Range;
  typename T::Value;
  // CConstBidirectionalRangeFromPointImpl<typename T::Range>;
} &&
requires (T image) {
  { image.range() } -> ISBASICALLY<typename T::Range>;
};

#undef ISBASICALLY

} // namespace detail

template <typename T, typename = void>
struct CImage : std::false_type {};

template <typename T>
requires detail::CConstImageImpl<T>
struct CImage<T> : std::true_type {
  BOOST_CONCEPT_ASSERT((DGtal::concepts::CImage<T>));
};

} // namespace bridges

template <typename T>
concept CImage = bridges::detail::CImageImpl<T> && 
  CIsTrue<bridges::CConstImage<T>>;

} // namespace concepts
} // namespace DGtalTrace