#pragma once
#include "DGtal/base/Common.h"
#include "DGtal/base/BasicTypes.h"
#include <DGtal/helpers/StdDefs.h>
#include <DGtal/kernel/ArithmeticConversionTraits.h>
#include <DGtal/kernel/PointVector.h>
#include <cstddef>
#include <iterator>
#include <type_traits>
#include "../SafePtrDiff.cpp"
#include "../concepts/DGtalConcepts.cpp"
#include "RectangleND.cpp"

// Random-access iterator and range over every integer point in an
// N-dimensional rectangle [lowerBound, upperBound] (inclusive),
// traversed in row-major order (last dimension varies fastest).
//
// TPoint must be a DGtal::PointVector specialization (or any type
// satisfying: static constexpr Dimension dimension, operator[],
// default- and copy-constructible).

namespace DGtalTrace {

template <typename TPoint> requires concepts::CPoint<TPoint>
class RectangleND_Iterator {
private:
  typedef RectangleND_Iterator<TPoint> Self;
public:
  // ── DGtal / STL traits ────────────────────────────────────────────────────
  typedef TPoint                           Point;
  typedef typename TPoint::Component       Component;
  typedef std::random_access_iterator_tag  iterator_category;
  typedef std::random_access_iterator_tag  iterator_concept;
  typedef Point                            value_type;
  typedef DGtal::SafePtrDiff               difference_type;
  typedef const Point*                     pointer;
  typedef const Point&                     reference;
  static constexpr DGtal::Dimension dimension = TPoint::dimension;

  // ── Construction ──────────────────────────────────────────────────────────
  RectangleND_Iterator() = default;

  RectangleND_Iterator(const RectangleND<TPoint> &aRect, difference_type anIndex = 0)
      : RectangleND_Iterator(aRect.lower(), aRect.upper(), anIndex) {}

  RectangleND_Iterator(const Point &aLower, const Point &aUpper, difference_type anIndex = 0)
      : myLower(aLower), myIndex(anIndex) {

    for (DGtal::Dimension i = 0; i < dimension; ++i) {
      myExtents[i] = static_cast<difference_type>(aUpper[i] - aLower[i]) + 1;
    }
    myStrides[dimension - 1] = 1;
    for (DGtal::Dimension i = dimension - 1; i > 0; --i)
      myStrides[i - 1] = myStrides[i] * myExtents[i];

    myVolume = myStrides[0] * myExtents[0];
    refreshCache();
  }

  // ── Dereference ───────────────────────────────────────────────────────────
  [[nodiscard]] reference operator*() const { return myCached; }
  [[nodiscard]] pointer operator->() const { return &myCached; }

  // ── Random access by offset ───────────────────────────────────────────────
  [[nodiscard]] value_type operator[](difference_type n) const {
    return indexToPoint(myIndex + n);
  }

  // ── Increment / decrement ─────────────────────────────────────────────────
  RectangleND_Iterator &operator++() {
    ++myIndex;
    refreshCache();
    return *this;
  }
  RectangleND_Iterator &operator--() {
    --myIndex;
    refreshCache();
    return *this;
  }
  RectangleND_Iterator operator++(int) {
    auto tmp = *this;
    ++*this;
    return tmp;
  }
  RectangleND_Iterator operator--(int) {
    auto tmp = *this;
    --*this;
    return tmp;
  }

  // ── Arithmetic ────────────────────────────────────────────────────────────
  RectangleND_Iterator &operator+=(difference_type n) {
    myIndex += n;
    refreshCache();
    return *this;
  }
  RectangleND_Iterator &operator-=(difference_type n) {
    myIndex -= n;
    refreshCache();
    return *this;
  }

  [[nodiscard]] friend RectangleND_Iterator operator+(RectangleND_Iterator it,
                                                      difference_type n) {
    return it += n;
  }
  [[nodiscard]] friend RectangleND_Iterator operator+(difference_type n,
                                                      RectangleND_Iterator it) {
    return it += n;
  }
  [[nodiscard]] friend RectangleND_Iterator operator-(RectangleND_Iterator it,
                                                      difference_type n) {
    return it -= n;
  }
  [[nodiscard]] friend difference_type
  operator-(const RectangleND_Iterator &a, const RectangleND_Iterator &b) {
    return a.myIndex - b.myIndex;
  }

  // ── Comparison ────────────────────────────────────────────────────────────
  [[nodiscard]] friend bool operator==(const RectangleND_Iterator &a,
                                       const RectangleND_Iterator &b) noexcept {
    return a.myIndex == b.myIndex;
  }
  [[nodiscard]] friend std::strong_ordering
  operator<=>(const RectangleND_Iterator &a,
              const RectangleND_Iterator &b) noexcept {
    return a.myIndex <=> b.myIndex;
  }

  // ── Utility ───────────────────────────────────────────────────────────────
  [[nodiscard]] difference_type flatIndex() const noexcept { return myIndex; }
  [[nodiscard]] difference_type volume() const noexcept { return myVolume; }

  // O(N) bijection: flat index → Point
  [[nodiscard]] value_type indexToPoint(difference_type k) const {
    Point p;
    for (DGtal::Dimension i = 0; i < dimension; ++i)
      p[i] = myLower[i] +
             static_cast<Component>((k / myStrides[i]) % myExtents[i]);
    return p;
  }

private:
  void refreshCache() {
    if (myIndex >= 0 && myIndex < myVolume)
      myCached = indexToPoint(myIndex);
  }

  Point myLower{};
  difference_type myExtents[dimension]{};
  difference_type myStrides[dimension]{};
  difference_type myIndex = 0;
  difference_type myVolume = 0;
  Point myCached{};
};

// ── Range wrapper
// ─────────────────────────────────────────────────────────────

template <typename TPoint> class RectangleND_Range {
public:
  using Iterator = RectangleND_Iterator<TPoint>;
  using Point = TPoint;

  RectangleND_Range(const Point &aLower, const Point &aUpper)
      : myBegin(aLower, aUpper, 0), myEnd(aLower, aUpper, myBegin.volume()) {}

  [[nodiscard]] Iterator begin() const { return myBegin; }
  [[nodiscard]] Iterator end() const { return myEnd; }
  [[nodiscard]] std::ptrdiff_t size() const { return myBegin.volume(); }

  // O(N) subscript: the k-th point in the rectangle
  [[nodiscard]] Point operator[](std::ptrdiff_t k) const { return myBegin[k]; }

private:
  Iterator myBegin;
  Iterator myEnd;
};
} // namespace DGtalTrace