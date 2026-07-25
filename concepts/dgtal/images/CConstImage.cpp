#pragma once
#include <DGtal/images/CTrivialConstImage.h>
#include <concepts>
#include <boost/concept_check.hpp>
#include <boost/concept/assert.hpp>
#include <boost/concept/requires.hpp>
#include <DGtal/images/CConstImage.h>
#include "../../BaseConcepts.cpp"
#include "../ranges/CConstBidirectionalRangeFromPoint.cpp"
#include "CTrivialConstImage.cpp"

namespace DGtalTrace {
namespace concepts {

namespace bridges {
namespace detail {

#define ISBASICALLY DGtalTrace::concepts::detail::IsBasically

template <typename T>
concept CConstImageImpl = CTrivialConstImageImpl<T> && 
requires {
  typename T::Domain;
  typename T::ConstRange;
} &&
requires (T image) {
  { image.domain() } -> ISBASICALLY<typename T::Domain>;
  { image.constRange() } -> ISBASICALLY<typename T::ConstRange>;
};

#undef ISBASICALLY

} // namespace detail

template <typename T, typename = void>
struct CConstImage : std::false_type {};

template <typename T>
requires detail::CConstImageImpl<T>
struct CConstImage<T> : std::true_type {
  BOOST_CONCEPT_ASSERT((DGtal::concepts::CConstImage<T>));
};

} // namespace bridges

template <typename T>
concept CConstImage = bridges::detail::CConstImageImpl<T> && 
  CIsTrue<bridges::CConstImage<T>>;

} // namespace concepts
} // namespace DGtalTrace