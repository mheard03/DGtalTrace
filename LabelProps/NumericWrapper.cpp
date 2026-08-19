#pragma once

/**
 * @file ColorWrapperCRTP.h
 *
 * @brief CRTP mixin that turns an arbitrary type into a "color-like"
 * type: implicitly convertible to DGtal::Color and DGtal::uint32_t,
 * with comparisons, clamped arithmetic, and streaming -- all derived
 * from two hooks that the wrapped type must supply itself:
 *
 *   DGtal::Color toColor() const;
 *   static Derived fromColor( const DGtal::Color & );
 *
 * The mixin does not care how Derived stores its data (a DGtal::Color,
 * a raw uint32_t, four floats, an unrelated third-party color type --
 * anything), only that it can produce/consume a DGtal::Color. The
 * uint32_t conversion is derived automatically via
 * Color::getRGBA()/Color::setRGBA(), so you get that for free.
 *
 * Usage:
 *
 *   class MyColor : public ColorWrapperCRTP<MyColor>
 *   {
 *   public:
 *     MyColor() = default;
 *     DGtal::Color toColor() const { ... }
 *     static MyColor fromColor( const DGtal::Color & c ) { ... }
 *   };
 *   static_assert( ColorConvertible<MyColor> );
 *
 * The static_assert is optional but recommended: put it right after
 * the class definition so a missing/misnamed hook fails immediately
 * with a clear message, rather than surfacing later as an obscure
 * error inside the CRTP base the first time an operator is used.
 */

#include <DGtal/io/Color.h>
#include <DGtal/base/BasicTypes.h>
#include <iostream>
#include <concepts>
#include "../concepts/dgtal/CLabel.cpp"




// ---------------------------------------------------------------------------
// Concept describing what a wrapped type must provide.
// ---------------------------------------------------------------------------
namespace DGtalTrace {

/*
template <typename T, typename TLabel>
concept NumericAssignable = concepts::CLabel<TLabel> &&
requires(const T& t, const TLabel& value) {
  { t = value } -> std::same_as<T&>;
};
*/
template <typename T>
concept Numeric = std::integral<T> || std::floating_point<T>;

template <typename T, typename TNumeric = DGtal::int32_t>
concept NumericAssignable = concepts::CLabel<T> && Numeric<TNumeric> && std::assignable_from<T, TNumeric>;


template <typename T>
struct NumericConverter {};

template <NumericAssignable T>
struct NumericConverter<T> {
  template <Numeric N>
  inline static T& fromNumber(T& t, const N& value) {
    t = value;
    return *t;
  };
  template <Numeric N>
  inline static N toNumber(const T& t) {
    return static_cast<N>(t);
  };
};

template <typename T>
requires 
concept CanConvertToNumber


template <typename T>
requires requires { typename NumericConverter<T>; }
class NumericWrapper {
private:
  typedef NumericConverter<T> Converter;
  T myValue;

public:
  NumericWrapper() : myValue() {} 
  NumericWrapper(const T& value) : myValue(value) {} 
  NumericWrapper& operator=(const T& value) { myValue = value; }

  operator T&() {
    return myValue;
  }
  operator T&() const {
    return myValue;
  }

  template <Numeric N>
  NumericWrapper& operator=(const T& value) { myValue = value; }

  template <Numeric N>
  NumericWrapper& operator=(const N& value);

  template <Numeric N>
  NumericWrapper& operator=(N&& value);
};
