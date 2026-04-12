#pragma once
#include <DGtal/base/Assert.h>
#include <DGtal/base/Common.h>
#include <DGtal/helpers/StdDefs.h>
#include <concepts>
#include <cstddef>
#include <exception>
#include <initializer_list>
#include <iterator>
#include <ranges>
#include <type_traits>


template <typename T, DGtal::Dimension N> requires (N > 0)
struct CartesianProductIndexIteratorND {
  typedef std::array<size_t, N> SizeArray;
  typedef T ValueType;

  CartesianProductIndexIteratorND(const SizeArray& rangeSizes) : myRangeSizes(rangeSizes), myStrides(getStrides(rangeSizes)), mySize(getSize(rangeSizes)) {}
  CartesianProductIndexIteratorND(const std::vector<size_t>& rangeSizes) : CartesianProductIndexIteratorND(getRangeSizes(rangeSizes)) {}
  CartesianProductIndexIteratorND(const CartesianProductIndexIteratorND& other) :
    myRangeSizes(other.myRangeSizes),
    myStrides(other.myStrides),
    mySize(other.mySize)
  {}

  virtual size_t getComponentValue(SizeArray& container, DGtal::Dimension k) const {
    return container[k];
  }
  virtual void setComponentValue(SizeArray& container, size_t componentValue, DGtal::Dimension k) const {
    container[k] = componentValue;
  }

  virtual SizeArray getValue(size_t index) const {
    SizeArray result;
    for (DGtal::Dimension k = 0; k < N; k++) {
      setComponentValue(result, index/myStrides[k] % myRangeSizes[k], k);
    }
    return result;
  }

  virtual size_t getIndex(SizeArray value) const {
    size_t result = 0;
    for (DGtal::Dimension k = 0; k < N; k++) {
      result += getComponentValue(value, k) * myStrides[k];
    }
    return result;
  }

  static SizeArray getRangeSizes(const std::vector<size_t>& inputRangeSizes) {
    if (inputRangeSizes.size() != N) throw std::invalid_argument("rangeSizes provided to CartesianProductIndexIteratorND constructor was the wrong length.");
    SizeArray sizes;
    std::copy_n(inputRangeSizes.begin(), N, sizes.begin());
    return sizes;
  }

  static constexpr SizeArray getStrides(const SizeArray &rangeSizes) {
    SizeArray strides;
    size_t current_stride = 1;
    for (DGtal::Dimension k = N-1; k >= 0; k--) {
      strides[k] = current_stride;
      current_stride *= rangeSizes[k];
    }
    return strides;
  }

  static constexpr size_t getSize(const SizeArray &rangeSizes) {
    size_t size = 1;
    for (DGtal::Dimension k = N-1; k >= 0; k--) {
      size *= rangeSizes[k];
    }
    return size;
  }

 private:
  const SizeArray myRangeSizes;
  const SizeArray myStrides;
  const size_t mySize;
 
};



/*
namespace detail {

template <typename ValueType, typename RangeType>
concept IsRangeOf = requires {
  typename std::ranges::range_value_t<RangeType>;
  std::ranges::random_access_range<RangeType>;
  std::ranges::sized_range<RangeType>;
  std::convertible_to<typename std::ranges::range_value_t<RangeType>, ValueType>;
  std::convertible_to<ValueType, typename std::ranges::range_value_t<RangeType>>;
};

template <typename ValueType, typename... RangeType>
concept AreRangesOf = (IsRangeOf<ValueType, RangeType> && ...);

} // namespace detail
// */