#pragma once
#include <concepts>
#include <boost/concept_check.hpp>
#include <boost/concept/assert.hpp>
#include <boost/concept/requires.hpp>
#include <DGtal/kernel/domains/CDomain.h>
#include "../BaseConcepts.cpp"

namespace DGtalTrace {
namespace concepts {

namespace bridges {
namespace detail {

#define ISBASICALLY DGtalTrace::concepts::detail::IsBasically

template <typename T>
concept CDomainImpl = requires {
  typename T::Domain;
  typename T::Space;
  typename T::Point;
  typename T::Vector;
  typename T::Integer;
  typename T::Size;
  typename T::Dimension;
  typename T::Predicate;
  typename T::ConstIterator;
} && requires (T domain, T::Point point) {
  { domain.lowerBound() } -> ISBASICALLY<typename T::Point>;
  { domain.upperBound() } -> ISBASICALLY<typename T::Point>;
  { domain.size() } -> std::same_as<typename T::Size>;
  { domain.isInside(point) } -> std::same_as<bool>;
  { domain.predicate() } -> ISBASICALLY<typename T::Predicate>;
  { domain.begin(point) } -> ISBASICALLY<typename T::ConstIterator>;
};

#undef ISBASICALLY

} // namespace detail

template <typename T, typename = void>
struct CDomain : std::false_type {};

template <typename T>
requires detail::CDomainImpl<T>
struct CDomain<T> : std::true_type {
  BOOST_CONCEPT_ASSERT((DGtal::concepts::CDomain<T>));
};

} // namespace bridges

template <typename T>
concept CDomain = bridges::detail::CDomainImpl<T> &&
  CIsTrue<bridges::CDomain<T>>;

} // namespace concepts
} // namespace DGtalTrace