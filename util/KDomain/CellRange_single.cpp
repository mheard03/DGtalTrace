#pragma once
#include <cstddef>
#include <exception>
#include <iterator>
#include <ranges>
#include <DGtal/base/Common.h>
#include <DGtal/helpers/StdDefs.h>
#include <stdexcept>
#include "../concepts/CCell.cpp"
#include "../concepts/CXCellularGridSpaceND.cpp"
#include "KRectangle.cpp"
#include "setKCoords_unsafe.cpp"
#include "CellIterator.cpp"

namespace DGtalTrace {
namespace detail {

template <concepts::CXXCell TCell>
class CellRange_single {
  
public:
  // typedef DGtal::Z2i::KSpace::Cell Cell;
  typedef TCell Cell;
  typedef typename Cell::CellularGridSpace KSpace;
  typedef CellIterator<Cell> const_iterator;
  typedef const_iterator iterator;
  typedef KRectangle<KSpace> KRect;

  explicit CellRange_single(const Cell &templateCell, DGtal::ConstAlias<KSpace> ks)
  requires concepts::CCellularGridSpaceND<KSpace>
      : myKPtr(&ks), myBounds(makeBounds(templateCell, ks)), myTemplateCell(templateCell) {}

  CellRange_single(const Cell &templateCell, const KRect& rect)
      : myKPtr(&detail::ksInfinite<KSpace>), myBounds(makeBounds(templateCell, rect)), myTemplateCell(templateCell) {}

  bool empty() const {
    return size() == 0;
  }

  size_t size() const {
    if (myBounds.isEmpty()) return 0;
    size_t result = 1;
    for (auto length : myBounds.extent()) {
      result *= length;
    }
    return result;
  }

  iterator begin() const { 
    if (empty()) return end();
    checkValid();
    return iterator(myKPtr, myBounds.template lower<TCell>(myTemplateCell), myBounds.template upper<TCell>(myTemplateCell)); 
  }
  iterator end()   const { return iterator(); }

  void selfDisplay(std::ostream& out) const {
    std::string strInvalid = (myBounds.isValid(*myKPtr)) ? "" : "INVALID ";
    std::string strSize = "[" + std::to_string(size()) + "] ";
    out << "[" << strInvalid << "CellRange_single" << strSize << " " << myBounds << "]";    
  }

  void checkValid() const {
    if (!myKPtr->isValid()) throw std::out_of_range("myKPtr refers to an invalid KXSpace");
    if (!myBounds.isValid(*myKPtr)) throw std::out_of_range("myBounds was invalid with respect to myKPtr");
  }

 private:
  const KSpace *myKPtr;
  const KRect myBounds;
  const TCell myTemplateCell;

  static KRect makeBounds(const Cell &templateCell, const KRect& rect) {
    // DGtal::trace.info() << "makeBounds(templateCell, rect) " << templateCell << ", " << rect << std::endl;
    return KRect(rect.first(templateCell), rect.last(templateCell));
  }
  static KRect makeBounds(const Cell &templateCell, const KSpace& ks) requires concepts::CCellularGridSpaceND<KSpace> {
    // DGtal::trace.info() << "makeBounds(templateCell, ks) " << templateCell << ", " << ks << std::endl;
    return makeBounds(templateCell, KRect(ks));
  }
  static KRect makeBounds(const KRect& rect) {
    // DGtal::trace.info() << "makeBounds(rect) " << rect << std::endl;
    return makeBounds(rect.template lower<Cell>(), rect, rect.defaultSign());
  }

};

template <concepts::CXXCell TCell>
std::ostream& operator<<(std::ostream& out, const CellRange_single<TCell>& object) {
  object.selfDisplay(out);
  return out;
}
} // namespace detail
} // namespace DGtalTrace

