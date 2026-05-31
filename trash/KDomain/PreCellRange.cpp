#pragma once
#include "../CellTypeTraits.cpp"
#include "../concepts/CCell.cpp"
#include "CellIterator.cpp"
#include <DGtal/base/Common.h>
#include <DGtal/helpers/StdDefs.h>
#include <DGtal/kernel/PointVector.h>
#include <DGtal/topology/CCellFunctor.h>
#include <algorithm>
#include <compare>
#include <functional>
#include <ranges>

namespace DGtalTrace {
namespace detail {

template <typename TCell>
  requires concepts::CXXCell<TCell>
class CellRange_Inner {
public:
  typedef TCell Cell;
  typedef Cell::CellularGridSpace KSpace;
  using iterator = CellIterator<TCell>;

  CellRange_Inner() : myKPtr(nullptr) {}

  CellRange_Inner(DGtal::ConstAlias<KSpace> ks, const Cell &lower,
                  const Cell &upper)
    requires concepts::CXCell<Cell>
      : myKPtr(&ks), myLower(lower), myUpper(upper) {}

  CellRange_Inner(const Cell &lower, const Cell &upper)
    requires concepts::CXPreCell<Cell>
      : myKPtr(nullptr), myLower(lower), myUpper(upper) {}

  iterator begin() const
    requires concepts::CXCell<Cell>
  {
    return iterator(myKPtr, myLower, myUpper);
  }
  iterator begin() const
    requires concepts::CXPreCell<Cell>
  {
    return iterator(myLower, myUpper);
  }
  iterator end() const { return iterator(); }

  bool operator==(const CellRange_Inner &other) const = default;

private:
  const KSpace *myKPtr;
  const Cell myLower, myUpper;
};

template <concepts::CXPreCell TCell>
class PreCellRange {
 public:
  typedef TCell Cell;
  typedef Cell::PreCellularGridSpace KPreSpace;
  typedef CellTypeTraits<Cell>::UPreSelf UPreCell;
  typedef CellTypeTraits<Cell>::SPreSelf SPreCell;
  typedef CellTypeTraits<Cell>::PreSelf PreSelf;
  typedef Cell::Point Point;
  typedef DGtal::NumberTraits<typename Point::Component> NT;
  typedef detail::CellRange_Inner<Cell> InnerRange;
  static constexpr DGtal::Dimension dimension = DimensionOf<TCell>;
  typedef std::array<UPreCell, 2> CellPair;

 private:
  PreCellRange() = default;

 public:
  static std::vector<PreSelf> allStartCells() {
    if constexpr (concepts::CUXCell<Cell>) {
      return allStartUPreCells();
    }

    if constexpr (concepts::CSXCell<Cell>) {
      typedef typename CellTypeTraits<Cell>::SPreSelf SPreCell;
      std::vector<SPreCell> result;
      for (auto &cell : allStartUPreCells()) {
        for (bool sign : {true, false}) {
          result.push_back(KPreSpace::signs(cell, sign));
        }
      }
      return result;
    }
  }

 private:
  template <concepts::CXXCell TTCell>
  static void makeUniqueByTopologyAndSign(std::vector<TTCell> &cells) {
    if (cells.size() <= 1)
      return;

    typedef std::pair<int, bool> TopoSign;
    auto getTopoSign = [](const TTCell &cell) -> TopoSign {
      return { xTopology(cell), xSign(cell) };
    };

    std::ranges::sort(cells, std::ranges::less(), getTopoSign);
    const auto ur = std::ranges::unique(cells);
    cells.erase(ur.begin(), ur.end());
  }
  
  static std::optional<CellPair> getAdjustedBounds(const UPreCell &startCell,
                                                   const CellPair &bounds) {
    return _getAdjustedBounds(startCell, bounds, nullptr);
  }

 private:
  static std::optional<CellPair> _getAdjustedBounds(const UPreCell &startCell,
                                                    const CellPair &bounds,
                                                    const KSpace *kPtr) {
    auto lower = xFirst(startCell, bounds[0], kPtr);
    auto upper = xLast(startCell, bounds[1], kPtr);
    if (!lower || !upper) return std::nullopt;
    
    if (kPtr != nullptr) {
      lower = nullIfOutside(kPtr, lower.value());
      upper = nullIfOutside(kPtr, upper.value());
    }
    if (!lower || !upper) return std::nullopt;

    if (DGtal::isUpper(xKCoords(lower.value()), xKCoords(upper.value()))) // equivalent to lte
      return std::nullopt; 

    return std::optional<CellPair>({ lower.value(), upper.value() });
  }

  static std::vector<typename CellTypeTraits<Cell>::UPreSelf> allStartUPreCells() {
    typedef typename CellTypeTraits<Cell>::UPreSelf UPreCell;

    const UPreCell startCell = KPreSpace::uSpel({0, 0});
    std::set<int> topologies = {xTopology(startCell)};
    std::vector<UPreCell> result = {startCell};

    for (auto face : KPreSpace::uFaces(startCell)) {
      const int topology = xTopology(face);
      if (topologies.contains(topology))
        continue;
      topologies.insert(topology);
      result.push_back(face);
    }
    return result;
  }

  static std::optional<Cell> xFirst(const UPreCell &cell,
                                    const UPreCell &lowerCell,
                                    const KSpace *kPtr) {
    Point pResult = xKCoords(cell);
    for (DGtal::Dimension k = 0; k < dimension; k++) {
      if (NT::odd(xKCoord(lowerCell, k)) != NT::odd(xKCoord(cell, k)))
        pResult[k] += 1;
    }
    std::optional<Cell> result(0);
    xSetKCoords(result.value(), pResult);
    if (kPtr != nullptr)
      result = nullIfOutside(kPtr, result.value());
    return result;
  }
  static std::optional<Cell> xLast(const UPreCell &cell,
                                   const UPreCell &upperCell,
                                   const KSpace *kPtr) {
    Point pResult = xKCoords(cell);
    for (DGtal::Dimension k = 0; k < dimension; k++) {
      if (NT::odd(xKCoord(upperCell, k)) != NT::odd(xKCoord(cell, k)))
        pResult[k] -= 1;
    }
    std::optional<Cell> result(0);
    xSetKCoords(result.value(), pResult);
    result = nullIfOutside(kPtr, result.value());
    return result;
  }
  inline static std::optional<TCell> nullIfOutside(const KSpace* kPtr,
                                                   const TCell &cell) {
    if constexpr (concepts::CXCell<TCell>) {
      return kPtr->cIsInside(xKCoords(cell)) ? cell : std::nullopt;
    }
    return cell;    
  }

  template <concepts::CXXCell TTCell>
  inline static Point xKCoords(const TTCell &cell) {
    using namespace DGtalTrace::concepts;
    if constexpr (CUXCell<TTCell>)
      return KPreSpace::uKCoords(cell);
    if constexpr (CSXCell<TTCell>)
      return KPreSpace::sKCoords(cell);
  }
  template <concepts::CXXCell TTCell>
  inline static TTCell::Point::Component xKCoord(TTCell &cell, const DGtal::Dimension k) {
    using namespace DGtalTrace::concepts;
    if constexpr (CUXCell<TTCell>)
      return KPreSpace::uKCoord(cell, k);
    if constexpr (CSXCell<TTCell>)
      return KPreSpace::sKCoord(cell, k);
  }
  template <concepts::CXXCell TTCell>
  inline static void xSetKCoords(TTCell &cell, const typename TTCell::Point &point) {
    using namespace DGtalTrace::concepts;
    for (DGtal::Dimension k = 0; k < DimensionOf<TTCell>; k++) {
      cell.preCell().coordinates[k] = point[k];
    }
  }
  template <concepts::CXXCell TTCell>
  inline static int xTopology(TTCell &cell) {
    using namespace DGtalTrace::concepts;
    if constexpr (CUXCell<TTCell>)
      return KPreSpace::uTopology(cell);
    if constexpr (CSXCell<TTCell>)
      return KPreSpace::sTopology(cell);
  }
  template <concepts::CXXCell TTCell>
  inline static bool xSign(TTCell &cell) {
    if constexpr (concepts::CSXCell<TTCell>)
      return KPreSpace::sSign(cell);
    return true;
  }
};

} // namespace detail
/*
template <class TCell>
class CellRange;

template <concepts::CXCell TCell>
class CellRange<TCell> : public detail::PreCellRange<DGtal::Z2i::KSpace::Cell> {
  typedef detail::PreCellRange<TCell> Base;
  typedef Base::InnerRange InnerRange;
  typedef Base::KSpace KSpace;
  typedef DGtal::Z2i::KSpace::Cell Cell;
  // typedef Base::Cell Cell;
  typedef Base::PreCell PreCell;
  typedef Base::CellPair CellPair;
  using Base::allStartCells;
  using Base::copyUniqueByTopologyAndSign;
  using Base::getAdjustedBounds;

 public:
  static InnerRange create(DGtal::ConstAlias<KSpace> ks,
                           const PreCell &startCell, const CellPair &bounds)
    requires concepts::CXCell<Cell>
  {
    auto adjBounds = getAdjustedBounds(ks, startCell, bounds);
    if (!adjBounds) return InnerRange();      
    return InnerRange(ks, adjBounds.first, adjBounds.last);
  }

  static InnerRange create(DGtal::ConstAlias<KSpace> ks,
                           const PreCell &startCell)
    requires concepts::CXCell<TCell>
  {
    return create(ks, startCell, ks->lowerCell(), ks->upperCell());
  }

  static auto create(DGtal::ConstAlias<KSpace> ks,
                     const std::vector<PreCell> &startCells,
                     const CellPair &bounds) 
    requires concepts::CXCell<TCell>
  {
    auto uniqueStartCells = Base::copyUniqueByTopologyAndSign(startCells);
    std::vector<InnerRange> ranges;
    for (auto const &cell : uniqueStartCells) {
      auto adjBounds = getAdjustedBounds(ks, cell, bounds);
      if (adjBounds)
        ranges.push_back(create(ks, cell, bounds));
    }
    auto result = ranges | std::views::join;
    return result;
  }

  static auto create(DGtal::ConstAlias<KSpace> ks,
                     const std::vector<PreCell> &startCells)
    requires concepts::CXCell<TCell>
  {
    return create(ks, startCells, { ks->lowerCell(), ks->upperCell() });
  }
  
  static auto create(DGtal::ConstAlias<KSpace> ks)
    requires concepts::CXCell<TCell>
  {
    return create(ks, allStartCells(), { ks->lowerCell(), ks->upperCell() });
  }
};
/*
 {
public:
  typedef TCell Cell;
  typedef CellTypeTraits<TCell>::PreSelf PreCell;
  typedef Cell::CellularGridSpace KSpace;
  typedef Cell::PreCellularGridSpace KPreSpace;
  typedef Cell::Point Point;
  typedef DGtal::NumberTraits<typename Point::Component> NT;
  typedef detail::CellRange_Inner<Cell> InnerRange;
  static constexpr DGtal::Dimension dimension = KSpace::dimension;

private:
  typedef std::array<PreCell, 2> CellPair;
  CellRange() = delete;

public:
  static InnerRange create(DGtal::ConstAlias<KSpace> ks,
                           const PreCell &startCell, const CellPair &bounds)
    requires concepts::CXCell<Cell>
  {
    auto adjBounds = getTrueBounds(startCell, bounds, &ks);
    if (!adjBounds) return InnerRange();      
    return InnerRange(ks, adjBounds.first, adjBounds.last);
  }

  static InnerRange create(DGtal::ConstAlias<KSpace> ks,
                           const PreCell &startCell)
    requires concepts::CXCell<TCell>
  {
    return create(ks, startCell, ks->lowerCell(), ks->upperCell());
  }

  static InnerRange create(const PreCell &startCell, const CellPair &bounds)
    requires concepts::CXPreCell<Cell>
  {
    std::optional<Cell> first = xFirst(startCell, bounds[0], nullptr);
    std::optional<Cell> last = xLast(startCell, bounds[1], nullptr);
    if (!first || !last)
      return InnerRange();
    return InnerRange(first.value(), last.value());
  }

  template <std::ranges::range OutRange>
  static OutRange create(DGtal::ConstAlias<KSpace> ks,
                         const std::vector<Cell> &startCells,
                         const CellPair &bounds) 
    requires concepts::CXCell<TCell>
  {
    startCells = copyUniqueByTopologyAndSign(startCells);
    std::vector<InnerRange> ranges;
    for (auto &cell : startCells) {
      auto adjBounds = getTrueBounds(cell, bounds, &ks);
      if (adjBounds)
        ranges.push_back(create(ks, cell, bounds));
    }
    auto result = ranges | std::views::join;
    return result;
  }

  template <std::ranges::range OutRange>
  static OutRange create(DGtal::ConstAlias<KSpace> ks,
                         const std::vector<Cell> &startCells)
    requires concepts::CXCell<TCell>
  {
    return create(ks, startCells, { ks->lowerCell(), ks->upperCell() });
  }

  template <std::ranges::range OutRange>
  static OutRange create(DGtal::ConstAlias<KSpace> ks)
    requires concepts::CXCell<TCell>
  {
    return create(ks, allStartCells(), { ks->lowerCell(), ks->upperCell() });
  }

  template <std::ranges::range OutRange>
  static OutRange create(DGtal::ConstAlias<KSpace> ks,
                         const std::vector<Cell> &startCells,
                         const CellPair &bounds) 
    requires concepts::CXPreCell<TCell>
  {
    startCells = copyUniqueByTopologyAndSign(startCells);
    std::vector<InnerRange> ranges;
    for (auto &cell : startCells) {
      auto adjBounds = getTrueBounds(cell, bounds, &ks);
      if (adjBounds)
        ranges.push_back(create(ks, cell, bounds));
    }
    auto result = ranges | std::views::join;
    return result;
  }

  template <std::ranges::range OutRange>
  static OutRange create(DGtal::ConstAlias<KSpace> ks,
                         const std::vector<Cell> &startCells)
    requires concepts::CXPreCell<TCell>
  {
    return create(ks, startCells, { ks->lowerCell(), ks->upperCell() });
  }

  template <std::ranges::range OutRange>
  static OutRange create(DGtal::ConstAlias<KSpace> ks)
    requires concepts::CXPreCell<TCell>
  {
    return create(ks, allStartCells(), { ks->lowerCell(), ks->upperCell() });
  }

  static std::vector<typename CellTypeTraits<Cell>::PreSelf> allStartCells() {
    if constexpr (concepts::CUXCell<Cell>) {
      return allStartUPreCells();
    }

    typedef typename CellTypeTraits<Cell>::SPreSelf SPreCell;
    std::vector<SPreCell> result;
    for (auto &cell : allStartUPreCells()) {
      for (bool sign : {true, false}) {
        result.push_back(KPreSpace::signs(cell, sign));
      }
    }
    return result;
  }

  template <concepts::CXXCell TTCell>
  static std::vector<TTCell>
  copyUniqueByTopologyAndSign(const std::vector<TTCell> &cells) {
    std::vector<TTCell> result(cells);
    makeUniqueByTopologyAndSign(result);
    return result;
  }

  template <concepts::CXXCell TTCell>
  static void makeUniqueByTopologyAndSign(std::vector<TTCell> &cells) {
    if (cells.size() <= 1)
      return;

    typedef std::pair<int, bool> TopoSign;
    auto getTopoSign = [](TTCell &cell) -> TopoSign {
      return {xTopology(cell),
              (concepts::CSXCell<Cell>) ? KPreSpace::sSign(cell) : true};
    };

    std::ranges::sort(cells, std::ranges::less(), getTopoSign);
    const auto ur = std::ranges::unique(cells);
    cells.erase(ur.begin(), ur.end());
  }

private:
  static std::vector<typename CellTypeTraits<Cell>::UPreSelf> allStartUPreCells() {
    typedef typename CellTypeTraits<Cell>::UPreSelf UPreCell;

    const UPreCell startCell = KPreSpace::uSpel({0, 0});
    std::set<int> topologies = {xTopology(startCell)};
    std::vector<UPreCell> result = {startCell};

    for (auto face : KPreSpace::uFaces(startCell)) {
      const int topology = xTopology(face);
      if (topologies.contains(topology))
        continue;
      topologies.insert(topology);
      result.push_back(face);
    }
    return result;
  }

  static bool isUpper(const PreCell &lhs, const PreCell &rhs) {
    return DGtal::isUpper(lhs.preCell().coordinates, lhs.preCell().coordinates);
  }

  static std::optional<CellPair> getTrueBounds(const PreCell &startCell,
                                               const CellPair &bounds,
                                               const KSpace *kPtr) {
    auto lower = xFirst(startCell, bounds[0]);
    auto upper = xLast(startCell, bounds[1]);
    if (kPtr != nullptr) {
      lower = nullIfOutside(&kPtr, lower);
      upper = nullIfOutside(&kPtr, upper);
    }
    if (!lower || !upper)
      return std::nullopt;
    if (isUpper(lower, upper)) // equivalent to lte
      return std::nullopt; 
    return {lower, upper};
  }

  // ----------------------- KSpace functions ---------------------------------
private:
  static std::optional<Cell> xFirst(const PreCell &cell,
                                    const PreCell &lowerCell,
                                    const KSpace *kPtr) {
    Point pResult = xKCoords(cell);
    for (DGtal::Dimension k = 0; k < dimension; k++) {
      if (NT::odd(xKCoord(lowerCell, k)) != NT::odd(xKCoord(cell, k)))
        pResult[k] += 1;
    }
    std::optional<Cell> result(0);
    xSetKCoords(result, pResult);
    if (kPtr != nullptr)
      result = nullIfOutside(&kPtr, result);
    return result;
  }
  static std::optional<Cell> xLast(const PreCell &cell,
                                   const PreCell &upperCell,
                                   const KSpace *kPtr) {
    Point pResult = xKCoords(cell);
    for (DGtal::Dimension k = 0; k < dimension; k++) {
      if (NT::odd(xKCoord(upperCell, k)) != NT::odd(xKCoord(cell, k)))
        pResult[k] -= 1;
    }
    std::optional<Cell> result(0);
    xSetKCoords(result, pResult);
    if (kPtr != nullptr)
      result = nullIfOutside(&kPtr, result);
    return result;
  }
  inline static std::optional<TCell> nullIfOutside(DGtal::ConstAlias<KSpace> ks,
                                                   const TCell &cell) {
    return ks->cIsInside(xKCoords(cell)) ? cell : std::nullopt;
  }
  inline static Point xKCoords(const TCell &cell) {
    using namespace DGtalTrace::concepts;
    if constexpr (CUXCell<TCell>)
      return KPreSpace::uKCoords(cell);
    if constexpr (CSXCell<TCell>)
      return KPreSpace::sKCoords(cell);
  }
  inline static void xKCoord(TCell &cell, const DGtal::Dimension k) {
    using namespace DGtalTrace::concepts;
    if constexpr (CUXCell<TCell>)
      return KPreSpace::uKCoord(cell, k);
    if constexpr (CSXCell<TCell>)
      return KPreSpace::sKCoord(cell, k);
  }
  inline static void xSetKCoords(TCell &cell, const Point &point) {
    using namespace DGtalTrace::concepts;
    if constexpr (CUXCell<TCell>)
      KPreSpace::uSetKCoords(cell, point);
    if constexpr (CSXCell<TCell>)
      KPreSpace::sSetKCoords(cell, point);
  }
  inline static int xTopology(TCell &cell) {
    using namespace DGtalTrace::concepts;
    if constexpr (CUXCell<TCell>)
      return KPreSpace::uTopology(cell);
    if constexpr (CSXCell<TCell>)
      return KPreSpace::sTopology(cell);
  }
};
*/
} // namespace DGtalTrace
