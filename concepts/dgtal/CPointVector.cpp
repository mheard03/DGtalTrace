#pragma once
#include <concepts>
#include <type_traits>
#include <DGtal/base/Assert.h>
#include <DGtal/base/Common.h>
#include <DGtal/kernel/PointVector.h>

namespace DGtalTrace {
namespace concepts {

template <typename T, DGtal::Dimension dim = 0>
concept CPointVector = requires {
  typename DGtal::IsAPointVector<T>;
  requires DGtal::IsAPointVector<T>::value;
  requires DGtal::IsAPointVector<T>::value == true;
  requires dim == 0 || T::dimension == dim;
};

namespace {
  static_assert(CPointVector<std::string> == false);
  static_assert(CPointVector<DGtal::PointVector<2, int>> == true);
  static_assert(CPointVector<DGtal::PointVector<2, int>, 2> == true);
  static_assert(CPointVector<DGtal::PointVector<2, int>, 3> == false);
}

}
}