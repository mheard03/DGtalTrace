
#pragma once
#include <concepts>
#include <iterator>
#include <ranges>

namespace DGtalTrace {

template <std::integral T>
class PiotaView : public std::ranges::view_interface<PiotaView<T>> {
public:
  enum class Parity { EVEN = 0, ODD = 1, BOTH = 2 };

  struct iterator {
    using value_type = T;
    using difference_type = std::ptrdiff_t;
    using iterator_category = std::random_access_iterator_tag;
    using iterator_concept = std::random_access_iterator_tag;

    T current;
    const T step_ = 1;

    iterator(T value, T step) : current(value), step_(step) {}

    constexpr T operator*() const { return current; }

    constexpr iterator& operator++()    { current += step_; return *this; }
    constexpr iterator  operator++(int) { auto tmp = *this; ++*this; return tmp; }
    constexpr iterator& operator--()    { current -= step_; return *this; }
    constexpr iterator  operator--(int) { auto tmp = *this; --*this; return tmp; }

    constexpr iterator& operator+=(difference_type n) { current += n * step_; return *this; }
    constexpr iterator& operator-=(difference_type n) { current -= n * step_; return *this; }

    constexpr iterator operator+(difference_type n) const { return {current + n * step_, step_}; }
    constexpr iterator operator-(difference_type n) const { return {current - n * step_, step_}; }
    friend constexpr iterator operator+(difference_type n, iterator i) { return i + n; }

    constexpr difference_type operator-(const iterator& rhs) const {
        return (current - rhs.current) / step_;
    }

    constexpr T operator[](difference_type n) const { return current + n * step_; }

    constexpr bool operator==(const iterator&) const = default;
    constexpr auto operator<=>(const iterator&) const = default;
  };

  constexpr PiotaView() = default;
  constexpr PiotaView(T start, T stop, const Parity parity)
      : myStart(fixParity(start, parity)), myStop(fixParity(stop, parity)), myParity(parity) {
    assert (myStop >= myStart);
  }

  constexpr iterator begin() const { return iterator(myStart, step(myParity)); }
  constexpr iterator end() const { return iterator(myStop, step(myParity)); }

  constexpr std::size_t size() const {
    if (myParity == Parity::BOTH) {
      return static_cast<std::size_t>(myStop - myStart);
    }
    return static_cast<std::size_t>((myStop - myStart + 1) / 2);
  }

  const Parity parity() const {
    return myParity;
  }

private:
  const T myStart = 0;
  const T myStop = 0;
  const Parity myParity = Parity::BOTH;

  inline static T fixParity(const T value, const Parity parity) {
    switch (parity) {
      case Parity::EVEN: return (value % 2 == 0) ? value : value+1;
      case Parity::ODD:  return (value % 2 == 1) ? value : value+1;
      case Parity::BOTH: return value;      
    }
  }

  inline static T step(const Parity parity) {
    return (parity == Parity::BOTH) ? 1 : 2;
  }
};

// Deduction guide
template <std::integral T> PiotaView(T, T, T) -> PiotaView<T>;

} // namespace DGtalTrace