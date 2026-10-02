#pragma once
#include <DGtal/base/Assert.h>
#include <DGtal/base/BasicTypes.h>
#include <concepts>
#include <DGtal/kernel/NumberTraits.h>
#include <cstdint>
#include <initializer_list>
#include <limits>
#include <span>
#include <type_traits>
#include "../concepts/BaseConcepts.cpp"

namespace DGtalTrace {

namespace detail {

template <typename T, typename... Args>
concept CCanConstructByForwarding = requires (Args&&... args) {
  T{std::forward<Args>(args)...};
};

template<std::unsigned_integral T, unsigned int Channels>
requires (Channels >= 1 && Channels <= 4)
struct Color {
public: //protected:
  static inline constexpr unsigned int NumChannels = Channels;
  static inline constexpr bool HasAlpha = (Channels % 2 == 0);
  static inline constexpr bool IsRgb = (Channels >= 3);
  static inline constexpr T ZERO = std::numeric_limits<T>::min();
  static inline constexpr T MAX = std::numeric_limits<T>::max();

private:
  typedef std::array<T, Channels> Array;
  Array _values;

  typedef std::make_signed_t<T> ST;
  typedef std::array<ST, Channels> SignedArray;
  static Array unsign(const SignedArray& values) {
    Array result;
    std::transform(values.begin(), values.end(), result.begin(),
      [](ST v) { 
        ASSERT(v >= 0);
        return static_cast<T>(v);
      });
    return result;
  }

public:
  template <typename... Args>
  requires CCanConstructByForwarding<Array, Args...>
  Color(Args&&... args) : _values(Array{std::forward<Args>(args)...}) {}

  template <typename... Args>
  requires CCanConstructByForwarding<SignedArray, Args...> && (!CCanConstructByForwarding<Array, Args...>)
  Color(Args&&... args) : _values(unsign(SignedArray{std::forward<Args>(args)...})) {}
  /*
  Color(const std::span<const ST, NumChannels>& values) : _values(unsign(values)) {}
  Color(const std::initializer_list<const ST>& values) : _values(unsign(std::span<const ST, NumChannels>(values))) {}
  */


private:
  template <std::unsigned_integral TOut>
  static inline constexpr bool CanUpscaleTo = std::numeric_limits<T>::max() <= std::numeric_limits<T>::max();

  template <std::unsigned_integral TOut>
  requires requires { CanUpscaleTo<TOut> == true; }
  static inline constexpr TOut UpscaleFactor = [](){
    TOut one = static_cast<TOut>(1);
    TOut halfTOutMaxPlusOne = (std::numeric_limits<unsigned short>::max() >> one) + one;
    TOut maxPlusOne = static_cast<TOut>(std::numeric_limits<unsigned short>::max()) + one;
    return (halfTOutMaxPlusOne / maxPlusOne) / static_cast<TOut>(2);
  }();

  template <std::unsigned_integral TOut>
  static TOut UpscaleChannel(const T value) {
    constexpr = std::numeric_limits<T>::max() <= std::numeric_limits<T>::max();
  }

requires requires {
    std::numeric_limits<T>::max() <= std::numeric_limits<T>::max();
  }
  static TOut UpscaleChannel(const T value) {
    constexpr = std::numeric_limits<T>::max() <= std::numeric_limits<T>::max();
  }

public:
  template <std::unsigned_integral TOut>
  Color<TOut, NumChannels>& operator=(const Color& rhs) {
    int i = 0;
    int amt = 7;
    int j = i << amt;
    
      // 1. Check for self-assignment
      if (this == &rhs) {
          return *this;
      }

      // 2. Free existing resource
      delete data;

      // 3. Allocate and copy new resource (Deep Copy)
      data = new int(*rhs.data);

      // 4. Return *this to allow chaining
      return *this;
  }

};

void maion() {
  Color<uint32_t, 1> c = { 0u };
  Color<uint32_t, 4> c2 = { 0 };
  c2 = c;
}


}

}