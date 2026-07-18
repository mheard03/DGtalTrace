#include <ranges>
#include "util/dimsOf.cpp"
#include "util/KCoordsAdjuster.cpp"
#include "util/concepts/CCell.cpp"
//#include "util/concepts/CXCellularGridSpaceND.cpp"
//#include "util/KDomain/KRectangle.cpp"
//#include "util/KDomain/CellIterator.cpp"
//#include "util/KDomain/CellRange.cpp"
//#include "util/KDomain/CellRange_single.cpp"
//#include "util/KDomain/biota_view.cpp"
//#include "util/KCellRange/CoordIOTraits.cpp"
//#include "util/KCellRange/CellRange.cpp"
#include "util/traceHelper.cpp"
//#include "util/makeRange.cpp"
#include <DGtal/base/Common.h>
#include <DGtal/kernel/PointVector.h>
#include <DGtal/helpers/StdDefs.h>
#include "ArcTracker.cpp"

using namespace DGtal;
using namespace DGtalTrace;

template <DGtalTrace::concepts::CXXCell TCell, DGtalTrace::concepts::CXCellularGridSpaceND TKSpace>
void printCell(const TCell& cell, const TKSpace& ks, const std::string& prefix = "") {
  if (prefix.size() > 0) {
    trace.info() << prefix;
  }
  trace.info() << cell;
  if constexpr (DGtalTrace::concepts::CCellularGridSpaceND<TKSpace>) {
    trace.info() << " valid:" << ks.uIsValid(cell) << ", inside:" << ks.uIsInside(cell);
  }
  trace.info() << std::endl;
}

template <typename T>
struct AdaptedPoint;

template <DGtalTrace::concepts::CPoint T>
struct AdaptedPoint<T> {
  typedef T Point;
  typedef Point::Component Component;

  // AdaptedPoint() {}
  AdaptedPoint(Point& value) : myValue(value) {}
  AdaptedPoint(AdaptedPoint& value) = default;
  AdaptedPoint(Point&& value) : myValue(value) {}
  AdaptedPoint(AdaptedPoint&& value) = default;

  Component &operator[](size_t index) {
    return myValue[index];
  }
  const Component &operator[](size_t index) const {
    return myValue[index];
  }

  constexpr operator Point&() & { return myValue; }
  constexpr operator Point() const& { return myValue; }

 private:
  Point myValue;
};

template <DGtalTrace::concepts::CUCell T>
struct AdaptedPoint<T> {
  typedef T::Point Point;
  typedef T::Point::Component Component;
  typedef T::CellularGridSpace KSpace;

  // AdaptedPoint() {}
  AdaptedPoint(T& value, const KSpace& ks) : myValue(value), myKPtr(&ks) {}
  AdaptedPoint(AdaptedPoint& value) = default;
  AdaptedPoint(T&& value, const KSpace& ks) : myValue(value), myKPtr(&ks) {}
  AdaptedPoint(AdaptedPoint&& value) = default;

  class Proxy {
    private:
        AdaptedPoint<T>& wrapper;
        size_t index;

    public:
        Proxy(AdaptedPoint<T>& w, size_t idx) : wrapper(w), index(idx) {}

        // 1. Intercepts assignment (writing): my_wrapper[0] = "new value";
        Proxy& operator=(const Component new_value) {
            wrapper.myKPtr->uSetKCoord(wrapper.myValue, index, new_value);
            return *this;
        }

        // 2. Intercepts casting/reading: std::string val = my_wrapper[0];
        operator Component() const {
            return wrapper.myKPtr->uKCoord(index);
        }
    };


  Proxy operator[](size_t index) {
    return Proxy(*this, index);
  }
  const Component &operator[](size_t index) const {
    return myKPtr->uKCoord(index);
  }

  operator T&() & { return myValue; }
  operator T() const& { return myValue; }
  operator Point() const& { return myValue.preCell().coordinates; }

 private:
  T myValue;
  const KSpace* myKPtr;
};

template <typename T>
requires requires { sizeof(AdaptedPoint<T>); } && 
  requires (std::ostream out, T object) { out << object; }
std::ostream& operator<<(std::ostream& out, const AdaptedPoint<T>& object) {
  out << static_cast<T>(object);
  return out;
}


int main(int argc, char** argv) {
  
  typedef DGtal::Z3i::KSpace KSpace;
  typedef DGtal::Z3i::KPreSpace KPreSpace;
  typedef KCoordsAdjuster<KSpace> KCA;
  typedef KSpace::Cell Cell;
  typedef KSpace::PreCell PreCell;
  typedef KSpace::Point Point;
  typedef KSpace::Integer Integer;


  Point p0({1, 1, 1});
  trace.info() << "p0: " << p0 << std::endl;
  AdaptedPoint<Point> ap = p0;
  trace.info() << "ap before: " << ap << std::endl;
  ap[0] = 17;
  trace.info() << "ap after:  " << ap << std::endl;
  Point p1 = ap;
  trace.info() << "p0 after:  " << p0 << std::endl;
  trace.info() << "p1 after:  " << p1 << std::endl;
  trace.info() << std::endl;
  


  KSpace ksPeriodic;
  ksPeriodic.init({0,0,0}, {3,3,3}, KSpace::Closure::PERIODIC);  
  auto ks = ksPeriodic;

  Cell c0 = ksPeriodic.uCell({2, 2, 2});
  trace.info() << "c0: " << c0 << std::endl;
  AdaptedPoint<Cell> ac(c0, ks);
  trace.info() << "ac before: " << ac << std::endl;
  ac[0] = -5;
  trace.info() << "ac after:  " << ac << std::endl;
  Cell c1 = ac;
  trace.info() << "c0 after:  " << c0 << std::endl;
  trace.info() << "c1 after:  " << c1 << std::endl;
  Point pc1 = ac;
  trace.info() << "pc1 after: " << pc1 << std::endl;
  return 0;


  //typedef KRectangle<PreCell> KRect;
  //typedef CellRange<SCell> SCellRange;

  // auto ks = KSpace({0,0}, {3,5}, false);
  /*
  KSpace ksPeriodic;
  ksPeriodic.init({0,0,0}, {3,3,3}, KSpace::Closure::PERIODIC);
  
  auto ks = ksPeriodic;
  trace.info() << "ks: " << ks << std::endl;
  trace.info() << "  lowerCell: " << ks.lowerCell() << std::endl;
  trace.info() << "  upperCell: " << ks.upperCell() << std::endl; 
  
  for (auto cell : SCellRange(ks, 1, KSpace::NEG)) {
    trace.info() << cell << std::endl;
  }
    */

  /*
  typedef CoordIOTraits<Point> PointIO;
  Point coords = { 10, 11, 12 };
  trace.info() << "PointIO::get({il}, 2) " << PointIO::get({0, 1, 2}, 2) << std::endl;
  trace.info() << "PointIO::get(coords, 2) " << PointIO::get(coords, 2) << std::endl;  
  trace.info() << "PointIO::set(coords, 2) " << std::endl;
  PointIO::set(coords, 2, 22);
  trace.info() << "PointIO::get(coords, 2) " << PointIO::get(coords, 2) << std::endl;
  trace.info() << "PointIO::get(coords) " << PointIO::get(coords) << std::endl;
  trace.info() << "PointIO::set(coords) " << std::endl;
  PointIO::set(coords, {100, 101, 102});
  trace.info() << "PointIO::get(coords, 2) " << PointIO::get(coords, 2) << std::endl;
  trace.info() << "PointIO::get(coords) " << PointIO::get(coords) << std::endl;
  trace.info() << std::endl;

  typedef CoordIOTraits<PreCell> PreCellIO;
  PreCell preCell = KPreSpace::uCell({ 10, 11, 12 });  
  trace.info() << "PreCellIO::get({il}, 2) " << PreCellIO::get({0, 1, 2}, 2) << std::endl;
  trace.info() << "PreCellIO::get(coords, 2) " << PreCellIO::get(preCell, 2) << std::endl;  
  trace.info() << "PreCellIO::set(coords, 2) " << std::endl;
  PreCellIO::set(preCell, 2, 22);
  trace.info() << "PreCellIO::get(coords, 2) " << PreCellIO::get(preCell, 2) << std::endl;
  trace.info() << "PreCellIO::get(coords) " << PreCellIO::get(preCell) << std::endl;
  trace.info() << "PreCellIO::set(coords) " << std::endl;
  PreCellIO::set(preCell, {100, 101, 102});
  trace.info() << "PreCellIO::get(coords, 2) " << PreCellIO::get(preCell, 2) << std::endl;
  trace.info() << "PreCellIO::get(coords) " << PreCellIO::get(preCell) << std::endl;
  trace.info() << std::endl;
  
  typedef CoordIOTraits<SCell> SCellIO;
  KSpace ksPeriodic;
  ksPeriodic.init({0,0,0}, {5,5,5}, KSpace::Closure::PERIODIC);
  SCellIO cellIO(ksPeriodic);
  SCell cell = ksPeriodic.sCell(coords, KSpace::POS);
  trace.info() << "SCellIO::get({il}, 2) " << SCellIO::get({0, 1, 2}, 2) << std::endl;
  trace.info() << "SCellIO::get(coords, 2) " << SCellIO::get(cell, 2) << std::endl;  
  trace.info() << "cellIO.set(coords, 2) " << std::endl;
  cellIO.set(cell, 2, 22);
  trace.info() << "CellIO::get(coords, 2) " << SCellIO::get(cell, 2) << std::endl;
  trace.info() << "CellIO::get(coords) " << SCellIO::get(cell) << std::endl;
  trace.info() << std::endl;
  return 0;

    typedef CoordIOTraits<SCell> SCellIO;
*/
  /*
  typedef BiotaView<int> BV;

  for (auto parity : { BV::Parity::EVEN, BV::Parity::ODD, BV::Parity::BOTH }) {
    auto bv = BV(0, 5, parity);
    trace.info() << "Parity " << (int)parity << ", size " << bv.size() << ": ";
    for (auto i : bv) {
      trace.info() << i;
    }
    trace.info() << ". bv[1]: " << bv[1] << std::endl;
  }
*/
  /*
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
  return 0;

  KSpace ksInfinite;
  KSpace ksOpen({0,0}, {5,5}, KSpace::Closure::OPEN);
  KSpace ksClosed({0,0}, {5,5}, KSpace::Closure::CLOSED);
  KSpace ksPeriodic;
  ksPeriodic.init({0,0}, {5,5}, KSpace::Closure::PERIODIC);
  */
  /*
  for (auto ks : { ksInfinite, ksOpen, ksClosed, ksPeriodic }) {
    trace.info() << "ks: " << ks << std::endl;
    trace.info() << "  lowerCell: " << ks.lowerCell() << std::endl;
    trace.info() << "  upperCell: " << ks.upperCell() << std::endl;  

    Point pMin = ks.uKCoords(ks.lowerCell());
    Point pMax = ks.uKCoords(ks.upperCell());
    std::vector<Point> points = { pMin, (pMax - pMin) / 2, pMax };
    if (ks.isAnyDimensionPeriodic()) {
      points.push_back((pMax * 3) / 2);
    }

    for (const Point& kCoords : points) {
      Cell cell = ks.uCell(kCoords);
      Point dCoords = ks.uCoords(cell);
      Cell templateCell = ks.uLast(cell);
      trace.info() << "kCoords:  " << kCoords << std::endl;
      trace.info() << "cell:     " << cell << std::endl;
      trace.info() << "dCoords:  " << dCoords << std::endl;
      trace.info() << "template: " << templateCell << std::endl;  // I could use cell, but I want to prove the rebuild works with an arbitrary cell of the correct topology  
      trace.info() << "rebuilt:  " << ks.uCell(dCoords, templateCell) << std::endl;
      trace.info() << std::endl;
    }
  }
  */
  /*
  KSpace ks = ksPeriodic;
  trace.info() << "ks: " << ks << std::endl;
  trace.info() << "  lowerCell: " << ks.lowerCell() << std::endl;
  trace.info() << "  upperCell: " << ks.upperCell() << std::endl; 
*/
/*
  typedef DGtalTrace::detail::CellRange_single<KSpace::SCell> Range;
  typedef typename Range::KRect Rect;
  KSpace::SCell templateCell = ks.sCell({-5, -5}, false);
  auto r = Range(ks, Rect(ks), templateCell);
  for (auto c : r) {
    trace.info() << c << " ";
  }
  trace.info() << std::endl;
  */
/*
  trace.info() << "<PV17>: ";
  for (auto k : dimsOf<PointVector<17, Z2i::Integer>>()) {
    trace.info() << k << " ";
  }
  trace.info() << std::endl;
  
  trace.info() << "<KSpace>: ";
  for (auto k : dimsOf<KSpace>()) {
    trace.info() << k << " ";
  }
  trace.info() << std::endl;
  
  trace.info() << "dimsOf(ksClosed): ";
  for (auto k : dimsOf(ksClosed)) {
    trace.info() << k << " ";
  }
  trace.info() << std::endl;

  trace.info() << "periodicDimsOf(ksClosed): ";
  for (auto k : periodicDimsOf(ksClosed)) {
    trace.info() << k << " ";
  }
  trace.info() << std::endl;

  trace.info() << "nonPeriodicDimsOf(ksClosed): ";
  for (auto k : nonPeriodicDimsOf(ksClosed)) {
    trace.info() << k << " ";
  }
  trace.info() << std::endl;


  trace.info() << "dimsOf(ksPeriodic): ";
  for (auto k : dimsOf(ksPeriodic)) {
    trace.info() << k << " ";
  }
  trace.info() << std::endl;

  trace.info() << "periodicDimsOf(ksPeriodic): ";
  for (auto k : periodicDimsOf(ksPeriodic)) {
    trace.info() << k << " ";
  }
  trace.info() << std::endl;

  trace.info() << "nonPeriodicDimsOf(ksPeriodic): ";
  for (auto k : nonPeriodicDimsOf(ksPeriodic)) {
    trace.info() << k << " ";
  }
  trace.info() << std::endl;

  trace.info() << "nonPeriodicDimsOf(ksPeriodic): ";
  for (auto k : nonPeriodicDimsOf(ksPeriodic)) {
    trace.info() << k << " ";
  }
  trace.info() << std::endl;

  // typedef DGtalTrace::KRectangle<KSpace::PreCell> KRect;
*/
  /*
  auto midCell = ks.uCell((ks.lowerBound() + (ks.upperBound() - ks.lowerBound()))/2);
  auto midCellEnd = midCell;
  ks.uSetKCoords(midCellEnd, ks.uKCoords(midCell) - Point::diagonal(1));
  */

  /*
  trace.info() << "proper precells:        " << KRect(KPreSpace::uCell({0, 0}), KPreSpace::uCell({11, 11}).preCell()) << std::endl;
  trace.info() << "improper precells:      " << KRect(KPreSpace::uCell({3, 3}), KPreSpace::uCell({1, 1}).preCell()) << std::endl;
  // trace.info() << "proper precells + ks:   " << KRect(KPreSpace::uCell({0, 0}), KPreSpace::uCell({11, 11}).preCell(), ks) << std::endl;
  // trace.info() << "improper precells + ks: " << KRect(KPreSpace::uCell({0, 0}), KPreSpace::uCell({-1, -1}).preCell(), ks) << std::endl;
  trace.info() << "proper cells + ks:      " << KRect(ks.uCell({0, 0}), ks.uCell({11, 11}), ks) << std::endl;
  trace.info() << "improper cells + ks:    " << KRect(ks.uCell({3, 3}), ks.uCell({1, 1}), ks) << std::endl;
  trace.info() << "ks only:                " << KRect(ks) << std::endl;
  */
  // trace.info() << "KRect::create(ks): " << KRect::create(ks) << std::endl;
  // trace.info() << "KRect::create(midCell, midCellEnd): " << KRect::create(midCell, midCellEnd) << std::endl;
  return 0;
  /*
  auto midCell = ks.uCell((ks.lowerBound() + (ks.upperBound() - ks.lowerBound()))/2);
  auto midCellEnd = midCell;
  ks.uSetKCoords(midCellEnd, ks.uKCoords(midCell) - Point::diagonal(1));
  KRectangle<KSpace> kRectP(midCell, midCellEnd);

  for (auto rect : { kRect, kRectP }) {
    trace.info() << rect << ", extent " << rect.extent(ks) << std::endl;
    DGtalTrace::CellRange<Cell> cr(ks.uSpel({0, 0}), rect);
    trace.info() << cr << std::endl;
    int loopCount = 0;
    for (auto cell : cr) {
      trace.info() << cell; 
      loopCount++;
      if (loopCount > 500) {
        trace.info() << std::endl << "busted";
        break;
      }
    }    
    trace.info() << std::endl;
  }
  return 0;

  typedef DGtalTrace::CellIterator<SCell> CIS;
  CIS i(ks, ks.sCell({0, 0}, KSpace::NEG), ks.sCell({10, 10}, KSpace::NEG));
  CIS end;
  std::vector<SCell> sCells;
  std::copy(i, end, std::back_inserter(sCells));
  trace.info() << "naked: " << std::endl;
  for (auto cell : sCells) {
    trace.info() << cell; 
  }
  trace.info() << std::endl << std::endl; 



  DGtalTrace::detail::CellRange_single<Cell> crRect(ksClosed.uCell({1, 1}), kRect);
  trace.info() << "crRect: " << crRect << std::endl;
  for (auto cell : crRect) {
    trace.info() << cell; 
  }
  trace.info() << std::endl << std::endl; 

  const SCell cell1 = ks.sCell({1, 1}, KSpace::NEG);
  const SCell cell2 = ks.sCell({1, 1}, KSpace::POS);

  CellRange<SCell> crJoined({cell1, cell2}, ks);
  trace.info() << "crJoined: " << crJoined.size() <<  std::endl;
  for (auto cell : crJoined) {
    trace.info() << cell; 
  }
  trace.info() << std::endl << std::endl; 

  typedef KCoordsAdjuster<KSpace> KCA;
  const auto filter = [](const SCell& cell) -> bool { return KCA::dim(cell) == 1 && KCA::sign(cell) == KSpace::POS; };
  auto templateCells = CellRange<SCell>::allStartCells([](const SCell& cell) -> bool { return KCA::dim(cell) == 1 && KCA::sign(cell) == KSpace::POS; });

  CellRange<SCell> crJoined2(templateCells, ks);
  trace.info() << "crJoined2: " << crJoined2.size() <<  std::endl;
  for (auto cell : crJoined2) {
    trace.info() << cell; 
  }
  trace.info() << std::endl << std::endl; 
*/
  /*
  std::vector<DGtalTrace::detail::CellRange_single<SCell>> tmp = { crKN, crKP };

  CellRange crJoined({crKN, crKP});
  trace.info() << "crJoined: " << crJoined.size() <<  std::endl;
  for (auto cell : crJoined) {
    trace.info() << cell; 
  }
  trace.info() << std::endl << std::endl; 

  std::vector<std::vector<Cell>> tmp2;
  for (auto i = ks.min(0); i <= ks.max(0); i++) {
    std::vector<Cell> cols;
    for (auto j = ks.min(1); j <= ks.max(1); j++) {
      cols.push_back(ks.uCell({i, j}));
    }
    tmp2.push_back(cols);
  }
  CellRange crJoined2(tmp2);
  trace.info() << "crJoined2: " << crJoined2.size() << std::endl;
  for (auto cell : crJoined2) {
    trace.info() << cell; 
  }
  trace.info() << std::endl << std::endl; 



  trace.info() << "allStartCells<Cell>: " << std::endl;
  for (auto cell : CellRange<Cell>::allStartCells()) {
    trace.info() << cell; 
  }
  trace.info() << std::endl << std::endl; 
  
  trace.info() << "allStartCells<SCell>: " << std::endl;
  for (auto cell : CellRange<SCell>::allStartCells()) {
    trace.info() << cell; 
  }
  trace.info() << std::endl << std::endl; 

  trace.info() << "allStartCells<Cell>(1): " << std::endl;
  for (auto cell : CellRange<Cell>::allStartCells(1)) {
    trace.info() << cell; 
  }
  trace.info() << std::endl << std::endl; 
  
  trace.info() << "allStartCells<SCell>(1): " << std::endl;
  for (auto cell : CellRange<SCell>::allStartCells(1)) {
    trace.info() << cell; 
  }
  trace.info() << std::endl << std::endl; 
  */
}

