#pragma once
#include <DGtal/base/Common.h>
#include <DGtal/kernel/PointVector.h>
#include <DGtal/kernel/SpaceND.h>
#include <algorithm>
#include <array>
#include <DGtal/helpers/StdDefs.h>
#include <DGtal/base/Trace.h>
#include <DGtal/kernel/domains/CDomain.h>
#include <initializer_list>
#include <type_traits>
#include "traceHelper.cpp"

template <typename TPointVector>
struct CEVec : public TPointVector::Container {
  static_assert(DGtal::IsAPointVector<TPointVector>::value);
  typedef TPointVector PointVector;
  typedef typename PointVector::Container Container;
  typedef typename PointVector::Component Component;
  static constexpr DGtal::Dimension dimension = PointVector::dimension;  

  using Container::Container;
  constexpr CEVec() = default;
  constexpr CEVec(const std::initializer_list<Component>& list) : Container() {
    assert(list.size() <= dimension);
    std::copy(list.begin(), list.end(), this->begin());
  };
  constexpr CEVec(const Container& other) : Container(other) {}
  constexpr CEVec(const PointVector& pointVector) : Container() { 
    for (DGtal::Dimension i = 0; i < dimension; i++) {
      this->operator[](i) = pointVector[i];
    }
  }

  
  template <int U = dimension, typename std::enable_if_t<U == 2, bool> = true>
  constexpr CEVec(const Component x, const Component y) : Container({x,y}) {}

  template <int U = dimension, typename std::enable_if_t<U == 3, bool> = true>
  constexpr CEVec(const Component x, const Component y, const Component z) : Container({x,y,z}) {}
  
/*
  template <typename ... Vs, std::enable_if_t<sizeof...(Vs) >= 1 && sizeof...(Vs) <= dimension, bool> = true>
  constexpr CEVec(const Vs... values) : Container({values...}) {}
*/


  operator const PointVector() const { return PointVector(this->data()); }



  constexpr CEVec& operator+=(const CEVec& rhs) {
    for (DGtal::Dimension i = 0; i < dimension; i++) {
      this->operator[](i) += rhs[i];
    }
    return *this;
  }
  friend constexpr CEVec operator+(CEVec lhs, const CEVec& rhs) {
    lhs += rhs;
    return lhs;
  }

  constexpr CEVec& operator-=(const CEVec& rhs) {
    for (DGtal::Dimension i = 0; i < dimension; i++) {
      this->operator[](i) -= rhs[i];
    }
    return *this;
  }
  friend constexpr CEVec operator-(CEVec lhs, const CEVec& rhs) {
    lhs -= rhs;
    return lhs;
  }

  constexpr CEVec& operator*=(const Component scalar) {
    for (DGtal::Dimension i = 0; i < dimension; i++) {
      this->operator[](i) *= scalar;
    }
    return *this;
  }
  friend constexpr CEVec operator*(CEVec lhs, const Component scalar) {
    lhs *= scalar;
    return lhs;
  }
};

typedef CEVec<DGtal::Z2i::Point> CEVec2;
typedef CEVec<DGtal::Z3i::Point> CEVec3;

CEVec2 c({1,2});