#pragma once
#include <concepts>
#include <DGtal/base/Assert.h>
#include "../concepts/dgtal/CLabel.cpp"

namespace DGtalTrace {

template <concepts::CLabel T>
requires concepts::CIsTrue<std::is_default_constructible<T>>
struct ZeroAssignable {
private:
  T myValue;
public:
  ZeroAssignable() : myValue() {} 

  ZeroAssignable(const ZeroAssignable& other) : myValue(other.myValue) {} 
  ZeroAssignable& operator=(const ZeroAssignable& other) { 
    myValue = other.myValue; 
    return *this;
  }

  ZeroAssignable(const T& value) : myValue(value) {} 
  ZeroAssignable& operator=(const T& value) { 
    myValue = value; 
    return *this;
  }

  template <typename N>
  requires (std::integral<N> || std::floating_point<N>)
  ZeroAssignable(const N& value) {
    ASSERT_MSG(value == 0, "Value must be zero");
    myValue = T();
    return *this;
  }

  template <typename N>
  requires (std::integral<N> || std::floating_point<N>)
  ZeroAssignable& operator=(const N& value) {
    ASSERT_MSG(value == 0, "Value must be zero");
    myValue = T();
    return *this;
  }

  operator T&() {
    return myValue;
  }
  operator const T&() const {
    return myValue;
  }
};

}

template <DGtalTrace::concepts::CLabel T>
std::ostream& operator<<(std::ostream& os, const DGtalTrace::ZeroAssignable<T>& t) {
  os << static_cast<T>(t);
  return os;
}
