#pragma once
#include <algorithm>
#include <forward_list>
#include <iterator>
#include <DGtal/base/Common.h>
#include <DGtal/helpers/StdDefs.h>
#include <DGtal/topology/KhalimskySpaceND.h>
#include "CellIterator.cpp"
#include <vector>
#include <ranges>
#include "../faceUtil.cpp"

template <class TCell>
class MultiCellIterator {
  typedef CellIterator<Cell> CellIterator;
  // typedef CellIterator::KSpace KSpace;
  typedef DGtal::Z2i::KSpace KSpace;
  typedef KSpace::PreCellularGridSpace PKSpace;
  typedef KSpace::Cell Cell;
  typedef KSpace::PreCell PreCell;
  typedef CellIterator::iterator_category iterator_category;
  typedef CellIterator::value_type value_type;
  typedef CellIterator::difference_type difference_type;
  typedef CellIterator::pointer pointer;
  typedef CellIterator::reference reference;

 private:
  void init(DGtal::ConstAlias<KSpace> ks, std::vector<Cell>& cells) {
    std::transform(cells.begin(), cells.end(), std::back_inserter(myIterators),
                   [&](Cell cell) { return CellIterator(ks, cell); });
    skipDone();
  }

 public:
  MultiCellIterator() {};

  explicit MultiCellIterator(DGtal::ConstAlias<KSpace> ks, std::vector<Cell>& cells) {
    init(ks, cells);
  }

  reference operator*()  const { return  *myIterators[myCurrentIndex]; }
  pointer   operator->() const { return   myIterators[myCurrentIndex].operator->(); }

  MultiCellIterator& operator++() {
    ++myIterators[myCurrentIndex];
    if (myIterators[myCurrentIndex] == myFinalIterator) {
      ++myCurrentIndex;
      skipDone();
    }
    return *this;
  }

  MultiCellIterator operator++(int) {
    auto tmp = *this;
    ++(*this);
    return tmp;
  }

  bool operator==(const MultiCellIterator& o) const {
    if (isDone() && o.isDone()) return true;
    return isDone() == o.isDone() &&
           myCurrentIndex == o.myCurrentIndex &&
           myIterators == o.myIterators;
  }

  bool operator!=(const MultiCellIterator& o) const { return !(*this == o); }

private:
  std::vector<CellIterator> myIterators;
  std::size_t myCurrentIndex = 0;  
  CellIterator myFinalIterator = CellIterator();

  bool isDone() const { return myCurrentIndex >= myIterators.size(); }

  // Advance past any sub-iterators that are already at their end.
  void skipDone() {
    while (!isDone() && myIterators[myCurrentIndex] == myFinalIterator)
      ++myCurrentIndex;
  }

  std::vector<Cell> getRepresentativePreCells() {
    auto firstSpel = PKSpace::uSpel({0, 0});
    std::vector<PreCell> preCells = { firstSpel };

    auto faces = PKSpace::uFaces(firstSpel);
    std::move(std::begin(faces), std::end(faces), std::back_inserter(preCells));

    constexpr auto getTopology = [](auto& c) { return PKSpace::uTopology(c); };
    std::ranges::sort(preCells, std::ranges::less{}, getTopology);
    std::ranges::unique(preCells, getTopology);
    // todo: erase
  }

  std::vector<Cell> getCells(DGtal::ConstAlias<KSpace> ks) {
    return getCells(ks, std::views::iota(DGtal::Dimension(0), PKSpace::dimension + 1));
  }

  std::vector<Cell> getCells(DGtal::ConstAlias<KSpace> ks, DGtal::Dimension dim) {
      return getCells(ks, {dim});
  }

  std::vector<Cell> getCells(DGtal::ConstAlias<KSpace> ks, std::vector<DGtal::Dimension> dims) {
    auto unfilteredPreCells = getRepresentativePreCells();
    std::set<Cell> resultSet;

    for (auto dim : dims) {
      for (auto preCell : unfilteredPreCells) {
        if (PKSpace::uDim(preCell) == dim) {
          resultSet.insert(ks->uFirst(preCell));
        }
      }
    }

    return std::vector<Cell>(resultSet.begin(), resultSet.end());
  }  

};




