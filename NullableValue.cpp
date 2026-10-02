#pragma once
#include <concepts>
#include <optional>
#include "concepts/CIsOptional.cpp"

namespace DGtalTrace {

template <typename T>
requires (!concepts::CIsOptional<T>)
using Nullable = std::optional<std::optional<T>>;

template <typename T>
bool isAssigned(const Nullable<T>& n) {
  return n.has_value();
}

template <typename T>
bool isAssignedNull(const Nullable<T>& n) {
  return !n->has_value();
}

template <typename T>
bool isAssignedValue(const Nullable<T>& n) {
  return isAssigned(n) && !isAssignedNull(n);
}

template <typename T>
std::optional<T> value(const Nullable<T>& n) {
  ASSERT(isAssigned(n));
  return n.value();
}


/*
concept CIsNullable = concepts::CIsOptional<T> && 
                      concepts::CIsOptional<detail::UnwrapOptValue<T>> && 
                      !concepts::CIsOptional<detail::UnwrapOptValue<detail::UnwrapOptValue<T>>>;

template <typename T>
requires (!concepts::CIsOptional<T>)
struct Nullable {

private:
  typedef std::optional<T> Opt;
  typedef std::optional<Opt> OptOpt;
  OptOpt _value;

public:
  Nullable() = default;
  Nullable(const Nullable& other) = default;
  explicit Nullable(const Opt& t) : _value(OptOpt{t}) {}  
  
  template <typename U>
  requires std::assignable_from<T, U> && (!concepts::CIsOptional<U>)
  Nullable(const U& u) : Nullable(Opt{u}) {}

  inline bool isAssigned() const {
    return _value.has_value();
  }
  inline bool isAssignedNull() const {
    return !_value->has_value();
  }
  inline bool isAssignedValue() const {
    return isAssigned() && !isAssignedNull();
  }

  T value() {
    ASSERT(isAssignedValue());
    return _value.value().value();
  }
};
*/
}

