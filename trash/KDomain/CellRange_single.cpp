#pragma once
#include <DGtal/helpers/StdDefs.h>
#include <DGtal/kernel/domains/HyperRectDomain_Iterator.h>
#include <ranges>
#include <DGtal/base/Common.h>
#include <utility>
#include "../concepts/CCell.cpp"
#include "KRectangle.cpp"

namespace DGtalTrace {
namespace detail {

template <typename TRange, typename TCell = void>
concept CRangeOfCells = std::ranges::range<TRange> && 
  concepts::CXXCell<std::ranges::range_value_t<TRange>> && (
    std::same_as<TCell, void> || concepts::CXXCell<TCell>
  ) && (
    std::same_as<TCell, void> ||
    std::same_as<TCell, std::ranges::range_value_t<TRange>> ||
    concepts::CIsTrue<std::is_convertible<std::ranges::range_value_t<TRange>, TCell>>
  );

template <concepts::CXXCell TCell>
class CellRange_single {
 public:
  typedef DGtal::Z2i::KSpace::Cell Cell;
  // typedef TCell Cell;
  typedef typename Cell::CellularGridSpace KSpace;
  typedef typename Cell::Integer Integer;
  typedef typename Cell::Point Point;
  typedef KRectangle<typename Cell::PreCellularGridSpace::Cell> KRect;
  typedef std::ranges::iota_view<Integer, Integer> IotaView;
  typedef std::array<IotaView, DimensionOf<KSpace>> IotaViewArray;

 private:
  class CellMaker {
   public:
    // CellMaker() : myKPtr(nullptr), myTemplateCell(TCell(0)) {}
    CellMaker(const KSpace& ks, const TCell& templateCell) : myKPtr(&ks), myTemplateCell(templateCell) {}
    TCell operator()(auto dCoordsTuple) {
      const Point dCoords = std::make_from_tuple<Point>(dCoordsTuple);
      if constexpr (concepts::CUCell<TCell>) return myKPtr->uCell(dCoords, myTemplateCell);
      if constexpr (concepts::CSCell<TCell>) return myKPtr->sCell(dCoords, myTemplateCell);
    }
   private:
    const KSpace *myKPtr;
    const TCell myTemplateCell;
  };

  inline static constexpr IotaView emptyIotaView = IotaView(0, 0);
  const CellMaker myCellMaker;
  IotaViewArray myBounds;

  typedef decltype(std::apply(std::views::cartesian_product, myBounds) | std::views::transform(myCellMaker)) View;

  View myView;
/*
In template: no matching function for call to object of type 'const DGtalTrace::detail::CellRange_single<DGtal::SignedKhalimskyCell<2>>::(lambda at /home/mheard/source/DGtalTrace/util/KDomain/CellRange_single.cpp:39:41)'
clang(ovl_no_viable_object_call)

CellRange_single.cpp(43, 20): Error occurred here
main.cpp(100, 20): In instantiation of template class 'DGtalTrace::detail::CellRange_single<DGtal::SignedKhalimskyCell<2>>' requested here
CellRange_single.cpp(39, 41): Candidate function not viable: no overload of 'declval' matching 'IotaViewArray &' (aka 'std::array<std::ranges::iota_view<int, int>, DimensionOf<KSpace>> &') for 1st argument
CellRange_single.cpp(39, 41): Conversion candidate of type 'transform_view<views::all_t<std::ranges::cartesian_product_view<std::ranges::iota_view<int, int>, std::ranges::iota_view<int, int>>>, DGtalTrace::detail::CellRange_single<DGtal::SignedKhalimskyCell<2>>::CellMaker> (*)(IotaViewArray &, const CellMaker &)' (aka 'transform_view<std::ranges::cartesian_product_view<std::ranges::iota_view<int, int>, std::ranges::iota_view<int, int>>, DGtalTrace::detail::CellRange_single<DGtal::SignedKhalimskyCell<2, int>>::CellMaker> (*)(std::array<std::ranges::iota_view<int, int>, DimensionOf<KSpace>> &, const CellMaker &)')
*/
 public:
  CellRange_single(const KSpace& ks, const KRect& rect, const TCell& templateCell) : 
        myCellMaker(CellMaker(ks, templateCell)),
        myBounds(makeBounds(ks, rect, templateCell)),
        myView(makeView(myBounds, myCellMaker)) {}

  auto begin() {
    return myView.begin();
  }

  auto end() {
    return myView.end();
  }

 private:
  static IotaViewArray makeBounds(const KSpace& ks, const KRect& rect, const TCell& templateCell) {
    PreCell lowerCell = rect.first(templateCell);
    PreCell upperCell = rect.last(templateCell);

    // populate bounds
    IotaViewArray bounds;
    if (DGtal::isLower(upperCell, lowerCell)) {
      bounds.fill(emptyIotaView);
    }
    else {
      Point lowerDCoords = KPreSpace::uCoords(lowerCell);
      Point upperDCoords = KPreSpace::uCoords(upperCell);
      
      for (auto k : dimsOf(ks)) {
        bounds[k] = IotaView(lowerDCoords[k], upperDCoords[k] + 1);
      }
    }
    return bounds;
  }

  View /*static CRangeOfCells<TCell> auto */ makeView(IotaViewArray& bounds, const CellMaker& cellMaker) {
    return std::apply(std::views::cartesian_product, bounds) | std::views::transform(cellMaker);
  }
};

/*

template <concepts::CXXCell TCell>
class CellRange_single2 {
  
public:
  typedef DGtal::Z2i::KSpace::Cell Cell;
  // typedef TCell Cell;
  typedef CellTypeTraits<Cell>::UPreSelf PreCell;
  typedef typename Cell::CellularGridSpace KSpace;
  typedef typename Cell::PreCellularGridSpace KPreSpace;
  typedef typename Cell::Integer Integer;
  typedef KCoordsAdjuster<KSpace> KCA;
  typedef typename Cell::Point Point;
  typedef KRectangle<PreCell> KRect;
  typedef std::ranges::iota_view<Integer, Integer> IotaView;
  typedef std::array<IotaView, DimensionOf<KSpace>> IotaViewArray;

  inline static constexpr IotaView emptyIotaView = IotaView(0, 0);

  explicit CellRange_single2(const Cell &templateCell, DGtal::ConstAlias<KSpace> ks)
  requires concepts::CCellularGridSpaceND<KSpace>
      : myKPtr(&ks), myBounds(makeBounds(templateCell, ks)), myTemplateCell(templateCell) {}

  CellRange_single2(const Cell &templateCell, const KRect& rect, DGtal::ConstAlias<KSpace> ks)
      : myKPtr(&ks), myBounds(makeBounds(templateCell, rect)), myTemplateCell(templateCell) {}


  static IotaViewArray makeBounds(const KSpace& ks, const KRect& rect, const TCell& templateCell) {
    PreCell lowerCell = rect.first(templateCell);
    PreCell upperCell = rect.last(templateCell);

    // populate bounds
    IotaViewArray bounds;
    if (DGtal::isLower(upperCell, lowerCell)) {
      bounds.fill(emptyIotaView);
    }
    else {
      Point lowerDCoords = KPreSpace::uCoords(lowerCell);
      Point upperDCoords = KPreSpace::uCoords(upperCell);
      
      for (auto k : dimsOf(ks)) {
        bounds[k] = IotaView(lowerDCoords[k], upperDCoords[k] + 1);
      }
    }

    return bounds;
  }

  static Point makePoint(auto& val) {
    return std::make_from_tuple<Point>(val);
  }

  TCell makeCell(Point& dCoords) {    
    if constexpr (concepts::CUPreCell<TCell>) return KPreSpace::uCell(dCoords, myTemplateCell);
    if constexpr (concepts::CSPreCell<TCell>) return KPreSpace::sCell(dCoords, myTemplateCell);
    if constexpr (concepts::CUCell<TCell>) return ks.uCell(dCoords, myTemplateCell);
    if constexpr (concepts::CSCell<TCell>) return ks.sCell(dCoords, myTemplateCell);
  }

  static auto makeRange(const KSpace& ks, const KRect& rect, const TCell& templateCell) {
    IotaViewArray bounds = makeRange(ks, rect, templateCell);
    // auto makePoint = [](auto val) { return std::make_from_tuple<Point>(val); };
    auto product = std::apply(std::views::cartesian_product, bounds) | std::views::transform(makePoint);
    return product;
  }


  
  auto ks = KSpace({0,0}, {3,5}, false);
  auto kca = KCA(ks);
  IotaViews iv;
  for (auto k : dimsOf(ks)) {
    iv[k] = std::views::iota(kca.kCoord(ks.lowerCell(), k), kca.kCoord(ks.upperCell(), k) + 1);
  }

  std::cout << "did stuff" << std::endl;

  auto makePoint = [](auto val) { return std::make_from_tuple<Point>(val); };
  std::tuple<Integer, Integer> t(0,5);
  trace.info() << "mft: " << std::make_from_tuple<Point>(t) << std::endl;

  auto product = std::apply(std::views::cartesian_product, iv) | std::views::transform(makePoint) | std::views::transform([&ks](Point p) { return ks.uCell(p); });
  for (auto p : product) {
    trace.info() << p << " ";
  }
  trace.info() << std::endl;


  size_t size() const {
    IotaViews iv = {std::views::iota(1,2), std::views::iota(1,5)};
    auto v = std::ranges::views::cartesian_product(iv);
    
    auto v2 = std::
    size_t result = 1;
    DGtal::trace.info() << "<myBounds.extent(*myKPtr) " << myBounds.extent(*myKPtr) << "; ";
    for (auto length : myBounds.extent(*myKPtr)) {
      if (length <= 0) return 0;
      result *= length;      
    }
    DGtal::trace.info() << "<result " << result << ">";
    return result;
  }

  iterator begin() const { 
    if (size() == 0) return end();
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
  const DBounds myDBounds;
  const TCell myTemplateCell;

  static DBounds makeBounds(const Cell &templateCell, const KRect& rect) {
    PreCell lower = rect.first(templateCell);
    PreCell upper = rect.last(templateCell);
    return {KCA::coords(lower), KCA::coords(upper)}
    return Self({lower, upper});
  }
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
*/

template <concepts::CXXCell TCell>
std::ostream& operator<<(std::ostream& out, const CellRange_single<TCell>& object) {
  object.selfDisplay(out);
  return out;
}

} // namespace detail
} // namespace DGtalTrace

