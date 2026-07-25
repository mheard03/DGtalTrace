#pragma once
#include <concepts>
#include <boost/concept_check.hpp>
#include <boost/concept/assert.hpp>
#include <boost/concept/requires.hpp>
#include <DGtal/images/CTrivialConstImage.h>
#include "../../BaseConcepts.cpp"
#include "../CPointFunctor.cpp"
#include "../CLabel.cpp"
#include "../CDomain.cpp"

namespace DGtalTrace {
namespace concepts {

namespace bridges {
namespace detail {

#define ISBASICALLY DGtalTrace::concepts::detail::IsBasically

template <typename T>
concept CTrivialConstImageImpl = CPointFunctorImpl<T> && 
requires {
  typename T::Value;
  typename T::Domain;
} && CLabelImpl<typename T::Value> && CDomainImpl<typename T::Domain> && 
requires (T image) {
  { image.domain() } -> ISBASICALLY<typename T::Domain>;
};

#undef ISBASICALLY

} // namespace detail

template <typename T, typename = void>
struct CTrivialConstImage : std::false_type {};

template <typename T>
requires detail::CTrivialConstImageImpl<T>
struct CTrivialConstImage<T> : std::true_type {
  BOOST_CONCEPT_ASSERT((DGtal::concepts::CTrivialConstImage<T>));
};

} // namespace bridges

template <typename T>
concept CTrivialConstImage = requires {
  requires CIsTrue<bridges::CTrivialConstImage<T>>;
};

} // namespace concepts
} // namespace DGtalTrace