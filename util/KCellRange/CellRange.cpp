#include "../KCoordsAdjuster.cpp"
#include "../concepts/CCell.cpp"
#include "../concepts/CXCellularGridSpaceND.cpp"
#include <DGtal/base/Common.h>
#include <DGtal/kernel/NumberTraits.h>
#include <array>
#include <cassert>
#include <cstddef>
#include <ranges>

namespace DGtalTrace {

template <concepts::CXXCell TCell>
class CellRange : public std::ranges::view_interface<CellRange<TCell>> {
public:
  using Cell = TCell;
  using Component = typename TCell::Point::Component;
  using KSpace = Cell::CellularGridSpace;
  using KPreSpace = Cell::PreCellularGridSpace;
  using KCA = KCoordsAdjuster<KSpace>;
  using NT = DGtal::NumberTraits<Component>;
  inline static constexpr DGtal::Dimension n = DimensionOf<KSpace>;

  CellRange() = default;

 protected:
  template <concepts::CXXCell UCell>
  requires concepts::CHasKPreSpace<KPreSpace, UCell>
  CellRange(Cell lower, UCell upper, unsigned int dim, const KSpace* kPtr)
      : myLower(lower), myUpper(upper), myDim(dim), myKPtr(kPtr) {
    assert(dim <= n);
  }

 public:
  template <concepts::CXXCell UCell>
  CellRange(Cell lower, UCell upper, unsigned int dim, const KSpace& ks)
  requires concepts::CHasKPreSpace<KPreSpace, UCell>
      : CellRange(lower, upper, dim, &ks) {}

  template <concepts::CXXCell UCell>
  CellRange(Cell lower, UCell upper, unsigned int dim)
  requires concepts::CPreCellularGridSpaceND<KSpace>
      : CellRange(lower, upper, dim, nullptr) {}


  // -------------------------------------------------------------------------
  class iterator {
  public:
    using value_type = Cell;
    using difference_type = std::ptrdiff_t;
    using iterator_category = std::forward_iterator_tag;
    using iterator_concept = std::forward_iterator_tag;

    iterator() = default;

    iterator(Cell lower, Cell upper, unsigned int oddCount, KCA adjuster)
        : lower_(lower), upper_(upper), oddCount_(oddCount), kca_(adjuster), done_(false) {
      if (!canSatisfy(0, static_cast<int>(oddCount_)))
        done_ = true;
      else
        resetFrom(0, static_cast<int>(oddCount_));
    }

    const Cell &operator*() const noexcept { return cell_; }
    const Cell *operator->() const noexcept { return &cell_; }

    // Hot path: last coord always steps by 2 (parity fixed by prefix).
    // lastDimBound_ is cached by resetFrom; no recomputation needed here.
    iterator &operator++() noexcept {
      kca_.setKCoord(cell_, n-1, kca_.kCoord(cell_, n-1) + 2);
      if (kca_.kCoord(cell_, n-1) == lastDimBound_) [[unlikely]] {
        carryFrom(n-2);
      }
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
        return true;
      return cell_ == o.cell_;
    }

  private:
    // ------------------------------------------------------------------
    // The last reachable value at dim k with the given parity.
    //
    // Non-periodic: the largest value <= upper[k] with that parity.
    //
    // Periodic (period T, values [lower[k], lower[k]+T-1]):
    //   The parity that matches lower[k] gets ceil(T/2) values;
    //   the other parity gets floor(T/2).
    //   lastValue = first(parity) + 2 * (count - 1).
    [[nodiscard]] Component lastValue(int k, bool oddParity) const noexcept {
      if (!kca_.isSpacePeriodic(k)) {
        const Component u = kca_.kCoord(upper_, k);
        return u - (NT::odd(u) != oddParity ? 1 : 0);
      }

      const Component T = traits_.period(k);
      const bool lowerParity = NT::odd(kca_.kCoord(lower_, k));
      const Component first = (parity == 1) ? firstOdd(k) : firstEven(k);
      const Component count =
          (parity == lowerParity) ? (T + Component{1}) / Component{2} : T / Component{2};
      return first + Component{2} * (count - Component{1});
    }

    // ------------------------------------------------------------------
    void carryFrom(DGtal::Dimension k) noexcept {
      while (k >= 0) {
        int oddBefore = 0;
        for (int i = 0; i < k; ++i)
          oddBefore += static_cast<int>(kca_.kCoord(cell_, i) & 1);
        const int need = static_cast<int>(oddCount_) - oddBefore;

        // Try +1 (flips parity).
        {
          const Component v = kca_.kCoord(cell_, k) + 1;
          const int vParity = static_cast<int>(v & Component{1});
          if (v <= lastValue(k, vParity)) {
            const int need_rest = need - vParity;
            if (canSatisfy(k + 1, need_rest)) {
              kca_.setKCoord(cell_, k, v);
              resetFrom(k + 1, need_rest);
              return;
            }
          }
        }
        // Try +2 (same parity).
        {
          const Component v = kca_.kCoord(cell_, k) + 2;
          const int vParity = static_cast<int>(kca_.kCoord(cell_, k) & 1);
          if (v <= lastValue(k, vParity)) {
            const int need_rest = need - vParity;
            if (canSatisfy(k + 1, need_rest)) {
              kca_.setKCoord(cell_, k, v);
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
    void resetFrom(int start, int r) noexcept {
      for (int j = start; j < static_cast<int>(n); ++j) {
        const int slack = static_cast<int>(n) - 1 - j;
        const bool forceOdd = !hasEven(j);
        const bool forceEven = !hasOdd(j);
        const bool assignOdd = forceOdd || (!forceEven && r > slack);

        if (assignOdd) {
          kca_.setKCoord(cell_, j, firstOdd(j));
          --r;
        } else {
          kca_.setKCoord(cell_, j, firstEven(j));
        }
      }
      // Cache the last-dim bound. Parity of dim n-1 is now fixed until
      // the next carry, so this value won't change until resetFrom fires again.
      const Component lastCoord = kca_.kCoord(cell_, n - 1);
      lastDimBound_ = lastValue(static_cast<int>(n) - 1,
                                static_cast<int>(lastCoord & Component{1}));
    }

    // ------------------------------------------------------------------
    // Non-periodic: parity exists iff at least one value in [lower,upper] has
    // it. Periodic, period >= 2: both parities always exist. Periodic, period
    // == 1: only lower[k]'s own parity exists.
    [[nodiscard]] bool hasOdd(int j) const noexcept {
      if (traits_.isPeriodic(j))
        return traits_.period(j) >= 2 ||
               static_cast<bool>(kca_.kCoord(lower_, j) & 1);
      return firstOdd(j) <= kca_.kCoord(upper_, j);
    }

    [[nodiscard]] bool hasEven(int j) const noexcept {
      if (traits_.isPeriodic(j))
        return traits_.period(j) >= 2 ||
               !static_cast<bool>(kca_.kCoord(lower_, j) & 1);
      return firstEven(j) <= kca_.kCoord(upper_, j);
    }

    [[nodiscard]] Component firstOdd(int j) const noexcept {
      return kca_.kCoord(lower_, j) | 1;
    }

    [[nodiscard]] Component firstEven(int j) const noexcept {
      const Component v = kca_.kCoord(lower_, j);
      return v + (v & Component{1});
    }

    // ------------------------------------------------------------------
    Cell cell_;
    Cell lower_;
    Cell upper_;
    DGtal::Dimension oddCount_;
    Component lastDimBound_;
    KCA kca_;
    bool done_{true};
  };

  // -------------------------------------------------------------------------
  [[nodiscard]] iterator begin() const {
    return iterator(myLower, myUpper, myOddCount, myTraits);
  }

  [[nodiscard]] iterator end() const noexcept { return iterator{}; }

private:
  Cell myLower;
  Cell myUpper;
  DGtal::Dimension myDim;
  KSpace* myKPtr = nullptr;
};