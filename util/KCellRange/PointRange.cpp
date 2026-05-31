#include <array>
#include <cassert>
#include <cstddef>
#include <iostream>
#include <ranges>

namespace DGtalTrace {
template <size_t n> using PointND = std::array<int, n>;

template <std::size_t n>
class PointRange : public std::ranges::view_interface<PointRange<n>> {
public:
  using Point = PointND<n>;

  PointRange() = default;

  PointRange(Point lower, Point upper, unsigned int oddCount)
      : myLower(lower), myUpper(upper), myOddCount(oddCount) {
    assert(oddCount <= n);
  }

  // -------------------------------------------------------------------------
  class iterator {
  public:
    using value_type = Point;
    using difference_type = std::ptrdiff_t;
    using iterator_category = std::forward_iterator_tag;
    using iterator_concept = std::forward_iterator_tag;

    // Default-constructed == end sentinel.
    iterator() = default;

    iterator(Point lower, Point upper, unsigned int oddCount)
        : lower_(lower), upper_(upper), oddCount_(oddCount), done_(false) {
      if (!canSatisfy(0, static_cast<int>(oddCount_)))
        done_ = true;
      else
        resetFrom(0, static_cast<int>(oddCount_));
    }

    const Point &operator*() const noexcept { return p_; }
    const Point *operator->() const noexcept { return &p_; }

    // Hot path: last coordinate always steps by 2 (parity is locked by the
    // prefix until a carry resets it). Carries are O(n) but rare.
    iterator &operator++() noexcept {
      p_[n - 1] += 2;
      if (p_[n - 1] <= upper_[n - 1]) [[likely]]
        return *this;
      carryFrom(static_cast<int>(n) - 2);
      return *this;
    }

    iterator operator++(int) {
      iterator tmp = *this;
      ++(*this);
      return tmp;
    }

    bool operator==(const iterator &o) const noexcept {
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
        const int need = static_cast<int>(oddCount_) - oddBefore;

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
    unsigned int oddCount_{0};
    bool done_{true};
  };

  // -------------------------------------------------------------------------
  [[nodiscard]] iterator begin() const {
    return iterator(myLower, myUpper, myOddCount);
  }

  [[nodiscard]] iterator end() const noexcept {
    return iterator{};
  } // default-constructed = done

private:
  Point myLower{};
  Point myUpper{};
  unsigned int myOddCount{0};
};

/*
int main() {
  // 4D rectangle from {0,0,0,0} to {3,3,3,3}, exactly 2 odd coordinates

  using Point = PointND<3>;

  for (auto i : {0, 1, 2, 3}) {
    std::cout << std::endl << "i = " << i << std::endl;
    PointRange<3> range({0, 0, 0}, {5, 5, 5}, i);
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