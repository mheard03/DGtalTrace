#pragma once
#include <optional>
#include <DGtal/base/Common.h>
#include <DGtal/base/ConstAlias.h>
#include <DGtal/images/ConstImageAdapter.h>
#include <DGtal/base/BasicFunctors.h>
#include <DGtal/base/FunctorHolder.h>
#include <type_traits>
#include <unistd.h>
#include "../concepts/dgtal/images/CConstImage.cpp"


namespace DGtalTrace {

template <concepts::CConstImage TConstImage>
struct SafeConstImage : private TConstImage {
  typedef TConstImage Base;
  typedef SafeConstImage<TConstImage> Self;
  typedef Base::Domain Domain;
  typedef Base::Point Point;
  typedef std::optional<typename Base::Value> Value;
  typedef Base::ConstRange ConstRange;
  
  using Base::Base;
  using Base::domain;
  using Base::constRange;

  SafeConstImage(const Base& other) : Base(other) {}
  
  SafeConstImage& operator=(const SafeConstImage& other) {
    if (&other != this) {
      Base::operator=(other);
      defaultValue = other.defaultValue;
    }
    return *this;
  }

  SafeConstImage& operator=(const Base& other) {
    if (&other != this) {
      Base::operator=(other);
    }
    return *this;
  }

  Value operator()(const Point &aPoint) const {
    if (domain().isInside(aPoint))
      return static_cast<Value>(Base::operator()(aPoint));
    else
      return defaultValue;
  }

  inline void selfDisplay(std::ostream& out) const {
    out << "[SafeConstImage ";
    Base::selfDisplay(out);
    out << "]";
  }

  const Base& base() const { return static_cast<Base>(*this); };
  Base& base() { return static_cast<Base>(*this); };
  
  void setDefaultValue(Value aValue) {
    defaultValue = aValue;
  }

  Value getDefaultValue() const {
    return defaultValue;
  }

 private:
  Value defaultValue = std::nullopt;
};


}