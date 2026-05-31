#pragma once
#include <algorithm>
#include <cstddef>
#include <exception>
#include <initializer_list>
#include <iterator>
#include <numeric>
#include <optional>
#include <ranges>
#include <DGtal/base/Common.h>
#include <DGtal/helpers/StdDefs.h>
#include <stdexcept>
#include <string>
#include <vector>
#include "../concepts/CCell.cpp"
#include "../concepts/CXCellularGridSpaceND.cpp"
#include "KRectangle.cpp"
#include "setKCoords_unsafe.cpp"
#include "CellRange_single.cpp"
#include "KCoordsAdjuster.cpp"
#include "../makeRange.cpp"

namespace DGtalTrace {
namespace {

  // template <typename UCell, typename TCell = void>
  // concept CCellRange_CellInput = concepts::CXXCell<UCell> && (std::same_as<TCell, void> || std::same_as<typename TCell::PreCellularGridSpace, typename UCell::PreCellularGridSpace>);

  template <typename TRange, typename TCell = void>
  concept CCellRange_RangeInput = std::ranges::range<TRange> && 
    concepts::CXXCell<std::ranges::range_value_t<TRange>> && 
    (std::same_as<TCell, void> || std::same_as<TCell, std::ranges::range_value_t<TRange>> || std::same_as<typename CellTypeTraits<TCell>::PreSelf, std::ranges::range_value_t<TRange>>);

/*
  template <typename TRange, typename TCell = void>
  concept CRangeOfCells = std::ranges::range<TRange> && 
    concepts::CXXCell<std::ranges::range_value_t<TRange>> && (
      std::same_as<TCell, void> || concepts::CXXCell<TCell>
    ) && (
      std::same_as<TCell, void> ||
      std::same_as<TCell, std::ranges::range_value_t<TRange>> ||
      concepts::CIsTrue<std::is_convertible<std::ranges::range_value_t<TRange>, TCell>>
    );
  */

  using namespace DGtal::Z2i;

  static_assert((CCellRange_RangeInput<std::vector<KSpace::Cell>> == true));
  static_assert((CCellRange_RangeInput<std::vector<KSpace::Point>> == false));
  static_assert((CCellRange_RangeInput<std::vector<KSpace::Cell>, KSpace::Cell> == true));
  static_assert((CCellRange_RangeInput<std::vector<KSpace::SPreCell>, KSpace::Cell> == false));
  static_assert((CCellRange_RangeInput<std::vector<KSpace::Cell>, DGtal::Z3i::Cell> == false));  
} // end anonymous

bool tst = std::is_convertible_v<DGtal::Z2i::KSpace::SCell, DGtal::Z2i::KSpace::PreCell>; 
//detail::CCellRange_RangeInput<std::vector<DGtal::Z2i::KSpace::Cell>, DGtal::Z2i::KSpace::SPreCell>;


template <concepts::CXXCell TCell>
class CellRange  {
 private:
  // typedef DGtal::Z2i::KSpace::Cell Cell;
  typedef TCell Cell;
  typedef TCell::Point Point;
  typedef detail::CellRange_single<TCell> InnerRange;
  typedef Cell::CellularGridSpace KSpace;
  typedef Cell::PreCellularGridSpace KPreSpace;
  typedef CellTypeTraits<Cell>::PreSelf  PreSelf;
  // typedef CellTypeTraits<Cell>::UPreSelf UPreSelf;  
  // typedef CellTypeTraits<Cell>::SPreSelf SPreSelf;  

  const std::vector<InnerRange> myRanges;
  typedef decltype(myRanges | std::views::join) View;
  const View myJoinedRange;
  
  typedef std::array<PreSelf, 2> RangeBounds;

 protected:
  CellRange(const std::vector<InnerRange> &ranges)
      : myRanges(ranges), myJoinedRange(myRanges) {}

  struct SubRange {
    Cell templateCell;
    Point firstDCoord;
    Point lastDCoord;

  };

 public:


  template <typename TCells>
  requires CCellRange_RangeInput<TCells, Cell>
  CellRange(const TCells &templateCells, DGtal::ConstAlias<KSpace> ks)
      : CellRange(makeRanges(templateCells, rect, ks)) {}


  template <typename TCells>
  requires CCellRange_RangeInput<TCells, Cell>
  explicit CellRange(const TCells &templateCells, const KRect& rect, DGtal::ConstAlias<KSpace> ks = detail::ksInfinite<KSpace>)
      : CellRange(makeRanges(templateCells, rect, ks)) {}

  template <typename TCells>
  requires CCellRange_RangeInput<TCells, Cell>
  explicit CellRange(TCells &templateCells, const KRect& rect, DGtal::ConstAlias<KSpace> ks = detail::ksInfinite<KSpace>)
      : CellRange(makeRanges(templateCells, rect, ks)) {}

  CellRange(const std::initializer_list<Cell> &templateCells, const KRect& rect, DGtal::ConstAlias<KSpace> ks = detail::ksInfinite<KSpace>)
      : CellRange(std::vector<Cell>{templateCells}, rect, ks) {}

  CellRange(const Cell &templateCell, const KRect& rect, DGtal::ConstAlias<KSpace> ks = detail::ksInfinite<KSpace>)
      : CellRange({ templateCell }, rect, ks) {}

 private:
  static std::vector<InnerRange> makeRanges(std::ranges::range auto& templateCells, const KRect& rect, const KSpace& ks) {
    std::vector<InnerRange> result;
    for (auto cell : templateCells) {
      result.push_back(InnerRange(cell, rect, ks));
    }
    return result;    
  }

 private:
  RangeBounds 

 public:
  size_t size() const { 
    if (myRanges.size() == 0) return 0;
    size_t size = 0;    
    for (auto range : myRanges) {
      size += std::ranges::size(range);
    }
    return size;
  }

  std::ranges::iterator_t<View> begin() const { 
    checkValid();
    return myJoinedRange.begin(); 
  }
  std::ranges::iterator_t<View> end() const { 
    return myJoinedRange.end(); 
  }

  static std::vector<Cell> allStartCells() {
    std::vector<Cell> result;
    for (UPreSelf& cell : allStartUPreCells()) {
      if constexpr (concepts::CUXCell<Cell>) {
        result.push_back(myAdjuster.template convert<Cell>(cell));
      }
      if constexpr (concepts::CSXCell<Cell>) {
        for (typename KSpace::Sign sign : {true, false}) {
          result.push_back(myAdjuster.template convert<Cell>(cell, sign));
        }
      }
    }
    return result;
  }

  void selfDisplay(std::ostream& out) const {
    std::string strSize = "[" + std::to_string(size()) + "] ";
    out << "[CellRange" << strSize << " { ";
    for (int i = 0; i < myRanges.size(); i++) {
      if (i > 0) out << ", ";
      out << myRanges[i];
    }
    out << " }]";
  }

  void checkValid() const {
    for (int i = 0; i < myRanges.size(); i++) {
      try {
        myRanges[i].checkValid();
      }
      catch (std::out_of_range e) {
        std::string msg = "myRanges[" + std::to_string(i) + "]: " + e.what();
        throw std::out_of_range(msg);
      }
    }
  }

  template <typename TCellPredicate> requires std::predicate<TCellPredicate, Cell>
  static auto allStartCells(const TCellPredicate& filter) {
    return allStartCells() | std::views::filter(filter);
  }

 private:
  static std::vector<UPreSelf> allStartUPreCells() {  
    const UPreSelf startCell = KPreSpace::uSpel({0, 0});
    std::set<int> topologies = { KPreSpace::uTopology(startCell) };
    std::vector<UPreSelf> result = {startCell};

    for (auto face : KPreSpace::uFaces(startCell)) {
      const int topology = KPreSpace::uTopology(face);
      if (topologies.contains(topology))
        continue;
      topologies.insert(topology);
      result.push_back(face);
    }
    return result;
  }
};
/*
template <concepts::CXXCell TCell, typename TKsOrRect>
explicit CellRange(std::initializer_list<TCell>, TKsOrRect) -> CellRange<TCell>;
*/
/*
template <typename TInnerRange>
requires CCellContainerN<TInnerRange, 1>
explicit CellRange(TInnerRange) -> CellRange<std::ranges::range_value_t<TInnerRange>, TInnerRange>;

template <typename TInnerRange>
explicit CellRange(std::initializer_list<TInnerRange>) -> CellRange<std::ranges::range_value_t<TInnerRange>, TInnerRange>;

template <typename TOuterRange>
requires CCellContainerN<TOuterRange, 2>
explicit CellRange(TOuterRange) -> CellRange<std::ranges::range_value_t<std::ranges::range_value_t<TOuterRange>>, std::ranges::range_value_t<TOuterRange>>;

constexpr bool b1 = CCellContainerN<DGtalTrace::detail::CellRange_single<SCell>, 1>;
constexpr bool b2 = CCellContainerN<std::vector<DGtalTrace::detail::CellRange_single<SCell> >, 2>;
*/
/*
template <typename TRange>
requires CCellRangeN<TRange, 1>
class CellRange : public std::ranges::view_interface<CellRange<TRange>> {
 private:
  typedef std::ranges::range_value_t<TRange> InnerRange;
  typedef std::ranges::range_value_t<InnerRange> Cell;
  const std::vector<InnerRange> myRanges;
  typedef decltype(myRanges | std::views::join) join_type;
  const join_type myJoinedRange;
  typedef Cell::CellularGridSpace KSpace;

 public:
  CellRange(const TRange& ranges) : 
      myRanges(ranges.begin(), ranges.end()),
      myJoinedRange(myRanges | std::views::join) {}
  
  auto begin() const { return myJoinedRange.begin(); }
  auto end() const { return myJoinedRange.end(); }
  size_t size() const requires std::ranges::sized_range<InnerRange> || std::ranges::sized_range<join_type> { 
    if constexpr (std::ranges::sized_range<join_type>) {
      return std::ranges::size(myJoinedRange);
    }
    if constexpr (std::ranges::sized_range<InnerRange>) {
      size_t size = 0;
      for (auto range : myRanges) {
        size += std::ranges::size(range);
      }
      return size;
    }
  }

 private:
  // template <concepts::CXXCell TCell>
  // static std::vector<TCell>

};
*/
template <concepts::CXXCell TCell>
std::ostream& operator<<(std::ostream& out, const CellRange<TCell>& object) {
  object.selfDisplay(out);
  return out;
}

} // namespace DGtalTrace

