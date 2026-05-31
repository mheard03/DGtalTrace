#pragma once
#include <cassert>
#include <compare>
#include <cstddef>
#include <iterator>
#include "../concepts/CCell.cpp"
#include "DGtal/kernel/PointVector.h"

template <typename TCell>
requires DGtalTrace::concepts::CXXCell<TCell>
class KCoord_Iterator {
public:
  // ── DGtal / STL traits ────────────────────────────────────────────────────
  typedef TPoint Point;
  typedef typename TPoint::Component Component;
  static constexpr DGtal::Dimension dimension = TPoint::dimension;

  // Note: value_type and reference are both Point (by value) — this is a
  // proxy iterator. iterator_category stays random_access_iterator_tag.
  typedef std::random_access_iterator_tag iterator_category;
  typedef std::random_access_iterator_tag iterator_concept;
  typedef Point value_type;
  typedef std::ptrdiff_t difference_type;
  typedef Point reference; // by value — no cache needed

  // ── Construction ──────────────────────────────────────────────────────────
  RectangleND_Iterator() = default;

  RectangleND_Iterator(const Point &aLower, const Point &aUpper,
                       difference_type aIndex = 0)
      : myLower(aLower), myIndex(aIndex) {
    for (DGtal::Dimension i = 0; i < dimension; ++i) {
      assert(aUpper[i] >= aLower[i]);
      myExtents[i] = static_cast<difference_type>(aUpper[i] - aLower[i]) + 1;
    }
    myStrides[dimension - 1] = 1;
    for (DGtal::Dimension i = dimension - 1; i > 0; --i)
      myStrides[i - 1] = myStrides[i] * myExtents[i];

    myVolume = myStrides[0] * myExtents[0];
  }

  // ── Dereference (computes on the fly, no cache) ───────────────────────────
  [[nodiscard]] value_type operator*() const { return indexToPoint(myIndex); }

  // ── Random access by offset ───────────────────────────────────────────────
  [[nodiscard]] value_type operator[](difference_type n) const {
    return indexToPoint(myIndex + n);
  }

  // ── Increment / decrement ─────────────────────────────────────────────────
  RectangleND_Iterator &operator++() {
    ++myIndex;
    return *this;
  }
  RectangleND_Iterator &operator--() {
    --myIndex;
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
    return *this;
  }
  RectangleND_Iterator &operator-=(difference_type n) {
    myIndex -= n;
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

  [[nodiscard]] value_type indexToPoint(difference_type k) const {
    Point p;
    for (DGtal::Dimension i = 0; i < dimension; ++i)
      p[i] = myLower[i] +
             static_cast<Component>((k / myStrides[i]) % myExtents[i]);
    return p;
  }

private:
  Point myLower{};
  difference_type myExtents[dimension]{};
  difference_type myStrides[dimension]{};
  difference_type myIndex = 0;
  difference_type myVolume = 0;
  // no myCached
};

// ── Range wrapper (unchanged)
// ─────────────────────────────────────────────────

template <typename TPoint> class RectangleND_Range {
public:
  typedef RectangleND_Iterator<TPoint> ConstIterator;
  typedef ConstIterator Iterator;
  typedef TPoint Point;

  RectangleND_Range(const Point &aLower, const Point &aUpper)
      : myBegin(aLower, aUpper, 0), myEnd(aLower, aUpper, myBegin.volume()) {}

  [[nodiscard]] ConstIterator begin() const { return myBegin; }
  [[nodiscard]] ConstIterator end() const { return myEnd; }
  [[nodiscard]] ConstIterator cbegin() const { return myBegin; }
  [[nodiscard]] ConstIterator cend() const { return myEnd; }
  [[nodiscard]] std::ptrdiff_t size() const { return myBegin.volume(); }

  [[nodiscard]] Point operator[](std::ptrdiff_t k) const { return myBegin[k]; }

private:
  ConstIterator myBegin;
  ConstIterator myEnd;
};