#pragma once
#include <DGtal/base/ConstAlias.h>
#include <concepts>
#include <cassert>
#include <cstddef>
#include <ranges>
#include <DGtal/base/Common.h>
#include "../concepts/BaseConcepts.cpp"
#include "../concepts/CCell.cpp"
#include "../KCoordsAdjuster.cpp"
#include "KRectangle.cpp"

namespace DGtalTrace {
namespace detail {
template <concepts::CPoint Point>
class KCoordsIterator {
public:
  using value_type = Point;
  using difference_type = std::ptrdiff_t;
  using iterator_category = std::forward_iterator_tag;
  using iterator_concept = std::forward_iterator_tag;
  inline static constexpr DGtal::Dimension n = DimensionOf<Point>;

  // Default-constructed == end sentinel.
  KCoordsIterator() = default;

  KCoordsIterator(Point lower, Point upper, DGtal::Dimension targetWeight)
      : lower_(lower), upper_(upper), targetWeight(targetWeight), done_(false) {
    assert(targetWeight <= DimensionOf<Point>);
    if (!canSatisfy(0, static_cast<int>(targetWeight)))
      done_ = true;
    else
      resetFrom(0, static_cast<int>(targetWeight));
  }

  const Point &operator*() const noexcept { return p_; }
  const Point *operator->() const noexcept { return &p_; }

  // Hot path: last coordinate always steps by 2 (parity is locked by the
  // prefix until a carry resets it). Carries are O(n) but rare.
  KCoordsIterator &operator++() noexcept {
    p_[n - 1] += 2;
    if (p_[n - 1] <= upper_[n - 1]) [[likely]]
      return *this;
    carryFrom(static_cast<int>(n) - 2);
    return *this;
  }

  KCoordsIterator operator++(int) {
    KCoordsIterator tmp = *this;
    ++(*this);
    return tmp;
  }

  bool operator==(const KCoordsIterator &o) const noexcept {
    if (done_ != o.done_)
      return false;
    if (done_)
      return true; // both at end
    return p_ == o.p_;
  }

private:
  // ------------------------------------------------------------------
  // Carry from position k upward, then reset everything to the right.
  // We only need to try +1 and +2: feasibility depends only on parity,
  // not the actual value, so if both fail there's nothing left at k.
  void carryFrom(int k) noexcept {
    while (k >= 0) {
      int oddBefore = 0;
      for (int i = 0; i < k; ++i)
        oddBefore += (p_[i] & 1);
      const int need = static_cast<int>(targetWeight) - oddBefore;

      // +1 first (smaller value, flips parity)
      {
        const int v = p_[k] + 1;
        if (v <= upper_[k]) {
          const int need_rest = need - (v & 1);
          if (canSatisfy(k + 1, need_rest)) {
            p_[k] = v;
            resetFrom(k + 1, need_rest);
            return;
          }
        }
      }
      // +2 (same parity)
      {
        const int v = p_[k] + 2;
        if (v <= upper_[k]) {
          const int need_rest = need - (v & 1);
          if (canSatisfy(k + 1, need_rest)) {
            p_[k] = v;
            resetFrom(k + 1, need_rest);
            return;
          }
        }
      }
      --k;
    }
    done_ = true;
  }

  // ------------------------------------------------------------------
  // Can positions [k, n) collectively produce exactly r odd values?
  // Feasibility: r must lie in [minOdd, maxOdd] where minOdd counts
  // axes with no even value in range, maxOdd counts axes with ≥1 odd.
  [[nodiscard]] bool canSatisfy(int k, int r) const noexcept {
    if (r < 0 || r > static_cast<int>(n) - k)
      return false;
    int minOdd = 0, maxOdd = 0;
    for (int j = k; j < static_cast<int>(n); ++j) {
      if (!hasEven(j))
        ++minOdd;
      if (hasOdd(j))
        ++maxOdd;
    }
    return r >= minOdd && r <= maxOdd;
  }

  // ------------------------------------------------------------------
  // Set p_[start..n-1] to the lex-minimum assignment with exactly r odds.
  // Greedy: prefer even (lower start) unless forced odd or we'd run out
  // of remaining positions to absorb the outstanding odd quota.
  void resetFrom(int start, int r) noexcept {
    for (int j = start; j < static_cast<int>(n); ++j) {
      const int slack = static_cast<int>(n) - 1 - j; // positions after j
      const bool forceOdd = !hasEven(j);
      const bool forceEven = !hasOdd(j);
      const bool assignOdd = forceOdd || (!forceEven && r > slack);

      if (assignOdd) {
        p_[j] = firstOdd(j);
        --r;
      } else {
        p_[j] = firstEven(j);
      }
    }
  }

  // ------------------------------------------------------------------
  // Parity helpers — all branchless, correct for negative ints (two's
  // complement). Step-by-2 is safe because each axis's parity is fixed
  // for the entire inner loop; the +1 path in carryFrom handles the
  // rare case where a parity flip at a higher coordinate is needed.

  // First odd  >= lower_[j]: lower_[j] | 1
  // First even >= lower_[j]: lower_[j] + (lower_[j] & 1)

  [[nodiscard]] bool hasOdd(int j) const noexcept {
    return (lower_[j] | 1) <= upper_[j];
  }

  [[nodiscard]] bool hasEven(int j) const noexcept {
    return (lower_[j] + (lower_[j] & 1)) <= upper_[j];
  }

  [[nodiscard]] int firstOdd(int j) const noexcept { return lower_[j] | 1; }

  [[nodiscard]] int firstEven(int j) const noexcept {
    return lower_[j] + (lower_[j] & 1);
  }

  // ------------------------------------------------------------------
  Point p_{};
  Point lower_{};
  Point upper_{};
  DGtal::Dimension targetWeight{0};
  bool done_{true};
};

template <concepts::CXXCell TCell>
class CellIterator : KCoordsIterator<typename TCell::Point> {
public:
  using value_type = TCell;
  using difference_type = std::ptrdiff_t;
  using iterator_category = std::forward_iterator_tag;
  using iterator_concept = std::forward_iterator_tag;
  inline static constexpr DGtal::Dimension n = DimensionOf<TCell>;
  using Point = TCell::Point;
  using KSpace = TCell::CellularGridSpace;
  using KCA = KCoordsAdjuster<KSpace>;
  using PreCell = CellTypeTraits<TCell>::UPreSelf;

  // Default-constructed == end sentinel.
  CellIterator() = default;

  CellIterator(const KCA& adjuster, const TCell& lower, const PreCell& upper, DGtal::Dimension targetDim)
      : myAdjuster(&adjuster), myCoordsIterator(KCA::kCoords(lower), KCA::kCoords(upper), targetDim), current(0) {
    myAdjuster->setKCoords(current, *myCoordsIterator);
  }

  const TCell &operator*() const noexcept { return current; }
  const TCell *operator->() const noexcept { return &current; }

  // Hot path: last coordinate always steps by 2 (parity is locked by the
  // prefix until a carry resets it). Carries are O(n) but rare.
  CellIterator &operator++() noexcept {
    ++myCoordsIterator;
    myAdjuster->setKCoords(current, *myCoordsIterator);
    return *this;
  }

  CellIterator operator--(int) {
    --myCoordsIterator;
    myAdjuster->setKCoords(current, *myCoordsIterator);
    return *this;
  }

  bool operator==(const CellIterator &o) const noexcept {
    return myCoordsIterator == o.myCoordsIterator;
  }

private:
  const KCoordsAdjuster<typename TCell::CellularGridSpace>* myAdjuster;
  KCoordsIterator<Point> myCoordsIterator;  
  TCell current{};
};
} // namespace detail

template <concepts::CXXCell TCell>
class CellRange : public std::ranges::view_interface<CellRange<TCell>> {
public:
  typedef TCell Cell;
  typedef TCell::Point Point;
  typedef detail::CellIterator<TCell> iterator;
  inline static constexpr DGtal::Dimension cellDim = DimensionOf<TCell>;
  using KSpace = TCell::CellularGridSpace;
  using Sign = KSpace::Sign;
  using KCA = KCoordsAdjuster<KSpace>;
  using PreCell = iterator::PreCell;
  using KRect = KRectangle<PreCell>;

protected:
  CellRange(const KRect& bounds, const DGtal::Dimension targetDim, const KCA& adjuster, const Sign& sign)
      : myBounds(bounds), myTargetDim(targetDim), myAdjuster(adjuster), mySign(sign) {}

public:
  CellRange() = default;

  CellRange(const KRect& bounds, const DGtal::Dimension targetDim)
  requires concepts::CUPreCell<TCell>
      : CellRange(bounds, targetDim, KCA::ksInfinite(), KSpace::POS) {}

  CellRange(const KRect& bounds, const DGtal::Dimension targetDim, const KSpace::Sign sign)
  requires concepts::CSPreCell<TCell>
      : CellRange(bounds, targetDim, KCA::ksInfinite(), sign) {}

  CellRange(const KSpace ks, const DGtal::Dimension targetDim)
  requires concepts::CUCell<TCell>
      : CellRange(KRect(ks), targetDim, KCA(ks), KSpace::POS) {}

  CellRange(const KSpace ks, const DGtal::Dimension targetDim, const KSpace::Sign sign)
  requires concepts::CSCell<TCell>
      : CellRange(KRect(ks), targetDim, KCA(ks), sign) {}

  // -------------------------------------------------------------------------
  [[nodiscard]] iterator begin() const {
    TCell lower = myAdjuster.template convert<TCell>(myBounds.lowerCell(), mySign);
    PreCell upper = myBounds.upperCell();
    return iterator(myAdjuster, lower, upper, myTargetDim);
  }

  [[nodiscard]] iterator end() const noexcept {
    return iterator{};
  } // default-constructed = done

private:
  const KRect myBounds;
  const DGtal::Dimension myTargetDim;
  const KCoordsAdjuster<KSpace> myAdjuster;
  const KSpace::Sign mySign;

  static TCell makeTemplateCell(const KSpace::Sign sign) 
  requires concepts::CSXCell<TCell> {
    return KCA::template make<TCell>(Point::diagonal(), sign);
  }
};

/*
int main() {
  // 4D rectangle from {0,0,0,0} to {3,3,3,3}, exactly 2 odd coordinates

  using Point = PointND<3>;

  for (auto i : {0, 1, 2, 3}) {
    std::cout << std::endl << "i = " << i << std::endl;
    CellRange<3> range({0, 0, 0}, {5, 5, 5}, i);
    Point prev = *range.begin();
    for (const auto &p : range) {
      if (p[0] != prev[0]) {
        std::cout << std::endl;
      }
      std::cout << p[0] << ',' << p[1] << ',' << p[2] << "  ";
      prev = p;
    }
  }
  return 0;
}
*/
}