#pragma once
#include "../concepts/CCell.cpp"
#include "../util/CellTypeTraits.cpp"
#include "../util/KCoordsAdjuster.cpp"
#include <DGtal/helpers/StdDefs.h>
#include <ranges>
#include <vector>
#include <type_traits>

namespace DGtalTrace {
namespace views {
// namespace {
  template <std::ranges::range Range>
  using El = std::ranges::range_value_t<Range>;

  template <typename Range>
  concept CSCellRange = std::ranges::range<Range> && concepts::CSXCell<El<Range>>;
// } // end anonymous namespace


// --- Unsigned Cell View -----------------------------------------------------
template <CSCellRange Range>
constexpr auto unsignedCellView = std::views::transform([](const El<Range>& cell) { 
  return KCoordsAdjuster<typename El<Range>::CellularGridSpace>::unsigns(cell);
});


// --- Opposite-direction Cell View -------------------------------------------
template <CSCellRange Range>
constexpr auto oppCellView = std::views::transform([](const El<Range>& cell) {
  return KCoordsAdjuster<typename El<Range>::CellularGridSpace>::opp(cell);
});

struct DirectionalViewMaker {
  template <typename T>
  static T adjust(const T& t, const bool reversed) { return t; }

  template <>
  static int adjust<int>(const int& t, const bool reversed) {
    return t;
  }

  template <typename T>
  inline static auto adjustedReverseView = ([](const bool reversed){
    if constexpr (requires (T tt) { DirectionalViewMaker::adjust<T>(tt, false); }) {
      return std::views::transform([](const T& value) {
        return DirectionalViewMaker::adjust<T>(value, reversed);
      });
    }
    else {
      return std::views::transform([](const T& value) {
        return &value;
      });
    }
  })();  



  template <typename T>
  auto applyAdjustment(const T& t, const bool reversed) {

    auto naiveResult = std::views::iota(std::size_t{0}, n) |
      std::views::transform(
        [b = std::ranges::begin(r), reversed,n](std::size_t i) -> std::ranges::range_reference_t<Range> {
          return b[reversed ? n - 1 - i : i];
        });

    if constexpr (requires (T tt) { adjust<T>(tt, reversed); }) {
      return adjust<T>(t, reversed);
    }
    else {
      return &t;
    }
  }

  void test() {
    int i = 17;
    auto adjusted = adjust(i);

    std::string potato = "potato";
    auto adjusted2 = adjust(potato);

  }
};

auto dl = DirectionalViewMaker::defaultLambda<int>;

// --- Conditionally-reversed-direction View ----------------------------------
namespace detail {


  // --- Core range factory ---------------------------------------------------
  template <std::ranges::random_access_range Range>
    requires std::ranges::sized_range<Range>
  auto makeDirectionalView(const Range &r, bool reversed) {
    typedef KCoordsAdjuster<typename El<Range>::CellularGridSpace> KCA;
    auto n = std::ranges::size(r);
    auto naiveResult = std::views::iota(std::size_t{0}, n) |
      std::views::transform(
        [b = std::ranges::begin(r), reversed,n](std::size_t i) -> std::ranges::range_reference_t<Range> {
          return b[reversed ? n - 1 - i : i];
        });


  }

  // --- Pipeable adaptor --------------------------------------------------
  struct DirectionalViewAdapter {
    bool reversed;

    template <std::ranges::random_access_range R>
      requires std::ranges::sized_range<R>
    friend auto operator|(R &&r, DirectionalViewAdapter self) {
      return makeDirectionalView(r, self.reversed);
    }
  };
} // end anonymous namespace

auto directionalView(bool reversed) {
  return anon::DirectionalViewAdapter{reversed};
}

} // namespace views


void doit() {
typedef DGtal::Z2i::KSpace KSpace;
typedef KSpace::SCell SCell;
typedef KSpace::Point Point;

KSpace ks;
std::vector<SCell> v;

v.push_back(ks.sCell(Point(1,1)));
v.push_back(ks.sCell(Point(2,2)));
v.push_back(ks.sCell(Point(3,3)));
v.push_back(ks.sCell(Point(4,4)));

  // Basic forward / reverse via pipe
  auto fwd = v | DGtalTrace::views::directionalView(false);
  auto rev = v | DGtalTrace::views::directionalView(true);

  auto mfwd = DGtalTrace::views::anon::makeDirectionalView(v, true);
  bool type = views::CSCellRange<decltype(mfwd)>;
}

} // namespace DGtalTrace

/*
namespace DGtalTrace {
namespace detail {

namespace {
  template <std::ranges::range Range>
  using El = std::ranges::range_value_t<Range>;
} // end anonymous namespace


template <std::ranges::range Range>
requires concepts::CSXCell<El<Range>>
constexpr auto unsignedCellView = std::views::transform([](const El<Range>&
cell) { return KCoordsAdjuster<typename
El<Range>::CellularGridSpace>::unsigns(cell);
});

template <std::ranges::range Range>
requires concepts::CSXCell<El<Range>>
constexpr auto oppCellView = std::views::transform([](const El<Range>& cell)  {
  return KCoordsAdjuster<typename El<Range>::CellularGridSpace>::opp(cell);
});

namespace {
  template <std::ranges::random_access_range Range>
  requires std::ranges::sized_range<Range>
  auto makeNaiveDirectionalView(Range&& r, bool reverse) {
    typedef std::ranges::range_size_t<Range> Size;
    Size n = std::ranges::size(r);

    return std::views::iota(std::size_t{0}, n) |
      std::views::transform([&r, reverse, n](Size i) {
        return r[(reverse) ? (n-1-i) : i];
      });
  }

  struct NaiveDirectionalViewAdapter {
    bool reversed;
    template <std::ranges::random_access_range Range>
      requires std::ranges::sized_range<Range>
    friend auto operator|(Range&& r, NaiveDirectionalViewAdapter self) {
        return makeNaiveDirectionalView(r, self.reversed);
    }
};

auto directionalView(bool reversed) { return
NaiveDirectionalViewAdapter{reversed}; }

} // end anonymous namespace

template <typename Range>
requires (!concepts::CSXCell<El<Range>>)
auto directionalView(bool reversed) {
  return NaiveDirectionalViewAdapter{reversed};
}

template <typename Range>
requires concepts::CSXCell<El<Range>>
auto directionalView(bool reversed) {
  return NaiveDirectionalViewAdapter{reversed} | oppCellView<Range>;
}



template <typename Range>
requires requires (Range r) { directional_view(r, true); }
using directional_view_t = decltype(directional_view(std::declval<Range>(),
true));

} // end namespace detail
} // end namespace DGtalTrace

*/