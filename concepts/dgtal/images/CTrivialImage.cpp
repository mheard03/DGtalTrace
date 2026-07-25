#pragma once
#include <concepts>
#include <boost/concept_check.hpp>
#include <boost/concept/assert.hpp>
#include <boost/concept/requires.hpp>
#include <DGtal/images/CTrivialImage.h>
#include "../../BaseConcepts.cpp"
#include "CTrivialConstImage.cpp"

namespace DGtalTrace {
namespace concepts {

namespace bridges {
namespace detail {

#define ISBASICALLY DGtalTrace::concepts::detail::IsBasically

template <typename T>
concept CTrivialImageImpl = CTrivialConstImageImpl<T> && 
requires {
  typename T::ConstRange;
} &&
requires (T image) {
  { image.constRange() } -> ISBASICALLY<typename T::ConstRange>;
};

#undef ISBASICALLY

} // namespace detail

template <typename T, typename = void>
struct CTrivialImage : std::false_type {};

template <typename T>
requires detail::CTrivialImageImpl<T>
struct CTrivialImage<T> : std::true_type {
  BOOST_CONCEPT_ASSERT((DGtal::concepts::CTrivialImage<T>));
};

} // namespace bridges

template <typename T>
concept CTrivialImage = requires {
  requires CIsTrue<bridges::CTrivialImage<T>>;
};

} // namespace concepts
} // namespace DGtalTrace