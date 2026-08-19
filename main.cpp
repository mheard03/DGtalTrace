#include <DGtal/base/BasicTypes.h>
#include <DGtal/base/Common.h>
#include <DGtal/helpers/StdDefs.h>
#include <DGtal/images/ImageSelector.h>
#include <DGtal/io/Color.h>
#include <DGtal/topology/helpers/Surfaces.h>
#include <DGtal/geometry/curves/FreemanChain.h>
#include <DGtal/geometry/curves/GridCurve.h>
#include <DGtal/kernel/PointVector.h>
#include <DGtal/io/readers/STBReader.h>
#include <DGtal/io/boards/Board2D.h>
#include <DGtal/io/colormaps/GradientColorMap.h>
#include <optional>
#include <ranges>
#include <tuple>
#include <type_traits>
#include <numeric>
#include <variant>
#include <typeindex>

#include "concepts/CXCellularGridSpaceND.cpp"
#include "util/SafeConstImage.cpp"
#include "util/dimsOf.cpp"
#include "util/KCoordsAdjuster.cpp"
// #include "util/KCellNavigator.cpp"
#include "concepts/CCell.cpp"
//#include "concepts/CXCellularGridSpaceND.cpp"
//#include "util/KDomain/KRectangle.cpp"
//#include "util/KDomain/CellIterator.cpp"
//#include "util/KDomain/CellRange.cpp"
//#include "util/KDomain/CellRange_single.cpp"
//#include "util/KDomain/biota_view.cpp"
//#include "util/KCellRange/CoordIOTraits.cpp"
//#include "util/KCellRange/CellRange.cpp"

//#include "util/makeRange.cpp"



#include "util/SurfelFollowers.cpp"
#include "ArcTracker.cpp"
#include "util/SafeConstImage.cpp"
#include "util/ConstSurfelPredicate.cpp"
#include "FrontierOrEdgePredicate.cpp"
#include "LabelProps/LabelPropertyMap.cpp"
#include "LabelProps/ZeroAssignable.cpp"
#include "util/ConstImageValueAdapter.cpp"
#include "util/detail/CellDeltaHelper.cpp"
#include "util/KPreCellNavigator.cpp"
#include "util/tests/testCellDeltaHelper.cpp"
#include "util/tests/testKPreCellNavigator.cpp"
// #include "ArcSet/custom_views.cpp"

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

template <typename TKSpace>
void testSurfelFollowers() {
  typedef TKSpace KSpace;
  typedef KCoordsAdjuster<KSpace> KCA;
  typedef typename KSpace::Cell Cell;
  typedef typename KSpace::SCell SCell;
  typedef typename KSpace::Point Point;
  typedef typename KSpace::Integer Integer;

  KSpace ks;
  SCell surfel = ks.sCell(Point(1,0), true);
  trace.info() << ks.sIsSurfel(surfel) << std::endl;

  DGtalTrace::functors::TrueSurfelPredicate<KSpace> predicateT;
  DGtalTrace::functors::FalseSurfelPredicate<KSpace> predicateF;
  
  bool int2ext = false;
  trace.info() << "Starting at " << surfel << std::endl;

  std::vector<SCell> contour1;
  SurfelAdjacency<2> sAdj(int2ext);
  DGtal::Surfaces<KSpace>::track2DSurface(contour1, ks, sAdj, predicateT, surfel);
  trace.info() << "native chain " << contour1 << std::endl;

  std::vector<SCell> contour2;
  SurfelFollowers<KSpace> SF;
  SF.allOutbound(contour2, surfel, int2ext, predicateT);
  trace.info() << "allOutbound " << contour2 << std::endl;

  trace.info() << "next chain ";
  SCell firstSurfel = surfel;
  bool hasNext;
  do {
    trace.info() << surfel << " ";
    hasNext = SF.next(surfel, int2ext);
  }
  while (hasNext && surfel != firstSurfel);
  trace.info() << std::endl;

  trace.info() << "nextUnique chain ";
  surfel = firstSurfel;
  do {
    trace.info() << surfel << " ";
    hasNext = SF.nextUnique(surfel);
  }
  while (hasNext && surfel != firstSurfel);
  trace.info() << std::endl;


  trace.info() << "prev chain ";
  firstSurfel = surfel;
  bool hasPrev;
  do {
    trace.info() << surfel << " ";
    hasPrev = SF.prev(surfel, int2ext);
  }
  while (hasPrev && surfel != firstSurfel);
  trace.info() << std::endl;

  trace.info() << "prevUnique chain ";
  surfel = firstSurfel;
  do {
    trace.info() << surfel << " ";
    hasPrev = SF.prevUnique(surfel);
  }
  while (hasPrev && surfel != firstSurfel);
  trace.info() << std::endl;
}

template <DGtalTrace::concepts::CConstImage TImage>
bool testSingleLpm() {
  typedef DGtal::Color Key;
  typedef LabelPropertyMap<TImage, std::string, false> StringMap;
  StringMap map;

  std::string potato = "potato";
  std::string tomato = "tomato";
  std::string beans = "beans";
  map[Key::Black] = potato;
  map[Key::White] = tomato;
  map[Key::Red] = tomato;
  map[std::nullopt] = beans;

  bool success = true;
  success &= map[Key::Black] != map[Key::White];
  success &= map[Key::Black] != map[std::nullopt];
  success &= map[Key::White] == map[Key::White];
  success &= map[Key::White] == map[Key::Red];

  success &= map.hasContrast(Key::Black, Key::White);
  success &= map.hasContrast(Key::Black, std::nullopt);
  success &= !map.hasContrast(Key::White, Key::White);
  success &= !map.hasContrast(Key::White, Key::Red);

  success &= map.contrastLevel(Key::Black, Key::White) == ContrastLevels::FULL;
  success &= map.contrastLevel(Key::Black, std::nullopt) == ContrastLevels::FULL;
  success &= map.contrastLevel(Key::White, Key::White) == ContrastLevels::NONE;
  success &= map.contrastLevel(Key::White, Key::Red) == ContrastLevels::NONE;

  return success;
}


template <DGtalTrace::concepts::CConstImage TImage>
bool testMultiLpm() {
  typedef DGtal::Color Key;
  typedef LabelPropertyMap<TImage, std::string, true> StringSetMap;
  StringSetMap map;

  map[Key::Black].insert("potato");
  map[Key::White].insert("tomato");
  map[Key::Red].insert("tomato");
  map[Key::Blue].insert("potato");
  map[Key::Blue].insert("tomato");
  map[Key::Green].insert("beans");
  map[Key::Green].insert("tomato");
  map[Key::Purple] = {};
  map[Key::Cyan] = {};
  map[std::nullopt].insert("beans");

  bool success = true;
  success &= map[Key::Black] != map[Key::White];
  success &= map[Key::Black] != map[std::nullopt];
  success &= map[Key::White] == map[Key::White];
  success &= map[Key::White] == map[Key::Red];
  success &= map[Key::Black] != map[Key::Blue];
  success &= map[Key::Cyan] == map[Key::Purple];

  success &= map.hasContrast(Key::Black, Key::White);
  success &= map.hasContrast(Key::Black, std::nullopt);
  success &= !map.hasContrast(Key::White, Key::White);
  success &= !map.hasContrast(Key::White, Key::Red);
  success &= map.hasContrast(Key::Black, Key::Blue);
  success &= !map.hasContrast(Key::Cyan, Key::Purple);
  
  success &= map.contrastLevel(Key::Black, Key::Black) == ContrastLevels::NONE;
  success &= map.contrastLevel(Key::Purple, Key::Cyan) == ContrastLevels::NONE;
  success &= map.contrastLevel(Key::White, Key::Red) == ContrastLevels::NONE;
  success &= map.contrastLevel(Key::Purple, Key::Black) == ContrastLevels::FULL;
  success &= map.contrastLevel(Key::Black, Key::Blue) == ContrastLevels::PARTIAL;
  success &= map.contrastLevel(Key::Blue, Key::Black) == ContrastLevels::PARTIAL;
  success &= map.contrastLevel(Key::Black, Key::White) == ContrastLevels::FULL;
  success &= map.contrastLevel(Key::White, Key::Black) == ContrastLevels::FULL;
  success &= map.contrastLevel(Key::Green, Key::Blue) == ContrastLevels::PARTIAL;
  success &= map.contrastLevel(Key::Blue, Key::Green) == ContrastLevels::PARTIAL;
  return success;
}

struct ToString {
  template <typename T>
  std::string operator()(const T &value) const {
    std::stringstream m_sstr;
    m_sstr << "\"" << value << "\"";
    return m_sstr.str();
  }  
};

template <std::ranges::range R>
auto to_vector(R&& r) {
    std::vector<std::ranges::range_value_t<R>> v;

    // if we can get a size, reserve that much
    if constexpr (requires { std::ranges::size(r); }) {
        v.reserve(std::ranges::size(r));
    }

    // push all the elements
    for (auto&& e : r) {
        v.push_back(static_cast<decltype(e)&&>(e));
    }

    return v;
}
  template <std::ranges::range Range>
  using El = std::remove_cvref_t<std::ranges::range_value_t<Range>>;

std::vector<DGtal::Z2i::KSpace::Surfel> /*DGtal::GridCurve<DGtal::Z2i::KSpace>*/ fc2gc(const DGtal::Z2i::KSpace& ks, const DGtal::FreemanChain<DGtal::int32_t>& fc, const DGtal::Z2i::Point& startKCoords) {
  typedef DGtal::Z2i::KSpace KSpace;
  typedef KSpace::Point Point;
  typedef KSpace::Surfel Surfel;

  std::vector<Surfel> surfels;
  surfels.reserve(fc.size());

  // per DGtal::FreemanChain.ih, 0=E(false), 1=N(false), 2=W(t), 3=S(true)
  std::array<std::pair<Point, bool>, 4> stepInfo = {{
    { Point( 1,  0), false },
    { Point( 0,  1), false },
    { Point(-1,  0), true },
    { Point( 0, -1), true }
  }};
  char char0 = '0';

  Point displacement;
  for (char c : fc.chain) {
    auto os = stepInfo[c-char0];
    surfels.push_back(ks.sCell(startKCoords + displacement + os.first, os.second));
    displacement += (2 * os.first);
  }
  return surfels;
}


template <DGtalTrace::concepts::CXCell TCell>
void testCdhDeltas() {
  typedef typename TCell::CellularGridSpace KSpace;
  typedef typename KSpace::Point Point;
  typedef KCoordsAdjuster<KSpace> KCA;
  typedef DGtalTrace::detail::CellDeltaHelper<KSpace> CDH;

  KSpace ks;
  KCoordsAdjuster<KSpace> kca(ks);
  DGtalTrace::detail::CellDeltaHelper<KSpace> cdh(ks);
  ks.init(Point(0,0), Point(5,5), {KSpace::Closure::PERIODIC, KSpace::Closure::PERIODIC});
  
  auto makeCell = [&ks](auto p) { 
    if constexpr(std::same_as<TCell, typename KSpace::Cell>) return ks.uCell(p);
    if constexpr(std::same_as<TCell, typename KSpace::SCell>) return ks.sCell(p);
  };

  for (auto x : { 0, 5, 12 }) {
    for (auto y : { 0, 6, 12 }) {  
      const TCell startCell = makeCell(Point(x, y));
      for (auto dim : { 0, 1 }) {
        for (auto dir : { -2, -1, 1, 2 }) {
          Point endKCoords = Point(x,y);
          endKCoords[dim] += dir;
          if (!ks.cIsInside(endKCoords)) continue;

          const TCell endCell = makeCell(endKCoords);
          // trace.info() << "Cells: " << startCell << " -> " << endCell << std::endl;
          auto delta = cdh.getDelta(startCell, endCell);
          // trace.info() << "  " << cdh.getRawDelta(startCell, endCell) << " = " << delta;
          assert(dim == delta.dim);
          // trace.info() << ".";
          assert((dir > 0 && delta.sign) || (dir < 0 && !delta.sign));
          // trace.info() << ".";
          TCell copy = makeCell(startCell);
          kca.shift(copy, delta);
          assert(copy == endCell);
          // trace.info() << ".";
          kca.shift(copy, delta.opp());
          assert(copy == startCell);
          // trace.info() << "." << std::endl;
        }
      }
    }
  }
}


int main(int argc, char** argv) {
  typedef DGtal::Z2i::KSpace KSpace;
  typedef DGtal::Z2i::KPreSpace KPreSpace;
  typedef DGtal::Z2i::Domain Domain;
  typedef DGtal::Z2i::Point Point;
  typedef ImageSelector<Domain, DGtal::Color>::Type Image;
  typedef ArcTracker<KSpace, Image> Tracker;

  // DGtalTrace::detail::OrthogonalDirections<KSpace> OD;
  

  /*
  typedef KSpace::SCell SCell;
  KSpace ks;

  std::vector<SCell> v;
  v.push_back(ks.sCell(Point(1,1)));
  v.push_back(ks.sCell(Point(2,2)));
  v.push_back(ks.sCell(Point(3,3)));
  v.push_back(ks.sCell(Point(4,4)));

    // Basic forward / reverse via pipe
    auto fwd = v | DGtalTrace::views::directionalView(false);
    auto rev = v | DGtalTrace::views::directionalView(true);


    auto fixed = fwd | DGtalTrace::views::oppCellView<decltype(fwd)>;

    for (SCell x : fwd) std::cout << x << ' ';   // 1 2 3 4 5
    std::cout << '\n';
    for (SCell x : rev) std::cout << x << ' ';   // 5 4 3 2 1
    std::cout << '\n';
    for (SCell x : fixed) std::cout << x << ' ';   // 5 4 3 2 1
    std::cout << '\n';

    auto v2 = std::views::transform(v, DGtal::functors::Identity);
    // Chained with other adaptors
    auto chained = v
                 | DGtalTrace::views::directionalView(true)
                 | std::views::take(3);

    for (SCell x : chained) std::cout << x << ' ';  // 4
    std::cout << '\n';
    return 0;
    */
  /*
  {
    std::vector<int> v = { 1, 2, 3 };
    DGtal::trace.info() << "v:       " << v << std::endl;

    auto fwdv = v | DGtalTrace::detail::directionalView(false);
    DGtal::trace.info() << "fwd:     " << to_vector(fwdv) << std::endl;

    auto revv = DGtalTrace::detail::directionalView(v, true);
    DGtal::trace.info() << "rev:     " << to_vector(revv) << std::endl;

    auto revrevv = DGtalTrace::detail::directionalView(revv, true);
    DGtal::trace.info() << "revrev:  " << to_vector(revrevv) << std::endl;
  }
  DGtal::trace.info() << std::endl;
  KSpace ks;
  typedef El<std::vector<KSpace::SCell>> scellel;

  bool test = DGtalTrace::concepts::CXXCell<scellel>;
  {
    std::vector<KSpace::SCell> v;
    v.push_back(ks.sCell(Point(1,1)));
    v.push_back(ks.sCell(Point(2,2)));
    v.push_back(ks.sCell(Point(3,3)));
    v.push_back(ks.sCell(Point(4,4)));
    DGtal::trace.info() << "v:       " << v << std::endl;

    auto fwdv = DGtalTrace::detail::directionalView(v, false);
    DGtal::trace.info() << "fwd:     " << to_vector(fwdv) << std::endl;

    auto revv = DGtalTrace::detail::directionalView(v, true);
    DGtal::trace.info() << "rev:     " << to_vector(revv) << std::endl;

    auto revrevv = DGtalTrace::detail::directionalView(revv, true);
    DGtal::trace.info() << "revrev:  " << to_vector(revrevv) << std::endl;
  }
  return 0;
  */
  /*
  typedef ZeroAssignable<std::string> ZeroString;
  
  ZeroString z;
  trace.info() << "1 z: " << z << std::endl;
  z = "potato";
  trace.info() << "2 z: " << z << std::endl;

  ZeroString z2(z);
  trace.info() << "3 z2: " << z2 << std::endl;
  z2 = "tomato";
  trace.info() << "4 z2: " << z2 << std::endl;
  z2 = z;
  trace.info() << "5 z2: " << z2 << std::endl;
  z2 = 0;
  trace.info() << "6 z2: " << z2 << std::endl;
  z2 = 17;
  trace.info() << "7 z2: " << z2 << std::endl;
  return 0;
  */

  // DGtal::trace.info() << testSingleLpm<Image>() << std::endl;
  // DGtal::trace.info() << testMultiLpm<Image>() << std::endl;

  Image img = DGtal::STBReader<Image>::import("cali2.png");
/*
  ConstImageValueAdapter<Image, Image::Value> noopAdapter(img);
  ConstImageValueAdapter<Image, std::optional<Image::Value>> optAdapter(img);

  ToString ts;
  ConstImageValueAdapter<Image, std::string, ToString> strAdapter(img, ts);



  Point p = {-1, -1};
  trace.info() << "noopAdapter: " << noopAdapter(p) << std::endl;
  trace.info() << "optAdapter: " << optAdapter(p) << std::endl;
  trace.info() << "strAdapter: " << strAdapter(p) << std::endl;

  noopAdapter.setDefaultValue(DGtal::Color::Purple);
  optAdapter.setDefaultValue(DGtal::Color::Blue);
  strAdapter.setDefaultValue("butts");

  trace.info() << "noopAdapter: " << noopAdapter(p) << std::endl;
  trace.info() << "optAdapter: " << optAdapter(p) << std::endl;
  trace.info() << "strAdapter: " << strAdapter(p) << std::endl;

  for (auto& p : img.domain()) {
    auto value = img(p);

    if (value != noopAdapter(p)) throw "noopFail";
    if (value != optAdapter(p).value()) throw "optFail";
    if (ts(value) != strAdapter(p)) throw "strFail";
  }
*/
  //return 0;

  auto results = testCdhDeltaBasics();
  DGtal::trace.info() << results.size() << " problems." << std::endl;

  KPreSpace pks;
  results = testSurfelPointelMethods(pks);
  DGtal::trace.info() << results.size() << " problems." << std::endl;

  return 0;

  KSpace ks;
  typedef KSpace::Surfel Surfel;
  typedef KSpace::SCell SCell;
  typedef KSpace::Cell Cell;
  typedef KSpace::Point Point;
  KCoordsAdjuster<KSpace> kca(ks);

  testCdhDeltas<KSpace::Cell>();
  testCdhDeltas<KSpace::SCell>();

/*
  DGtalTrace::detail::CellDeltaHelper<KSpace> cdh(ks);
  ks.init(Point(0,0), Point(5,5), {KSpace::Closure::PERIODIC, KSpace::Closure::PERIODIC});
  for (auto x : { 0, 5, 12 }) {
    for (auto y : { 0, 6, 12 }) {  
      const TCell startCell = ks.uCell(Point(x, y));
      for (auto dim : { 0, 1 }) {
        for (auto dir : { -2, -1, 1, 2 }) {
          Point endKCoords = Point(x,y);
          endKCoords[dim] += dir;
          if (!ks.cIsInside(endKCoords)) continue;

          const TCell endCell = ks.uCell(endKCoords);
          // trace.info() << "Cells: " << startCell << " -> " << endCell << std::endl;
          auto delta = cdh.getDelta(startCell, endCell);
          // trace.info() << "  " << cdh.getRawDelta(startCell, endCell) << " = " << delta;
          assert(dim == delta.dim);
          // trace.info() << ".";
          assert((dir > 0 && delta.sign) || (dir < 0 && !delta.sign));
          // trace.info() << ".";
          TCell copy = kca.uCell(startCell);
          kca.shift(copy, delta);
          assert(copy == endCell);
          // trace.info() << ".";
          kca.shift(copy, delta.opp());
          assert(copy == startCell);
          // trace.info() << "." << std::endl;
        }
      }
    }
  }
    */
return 0;

  ks.init(img.domain().lowerBound(), img.domain().upperBound(), {KSpace::Closure::PERIODIC, KSpace::Closure::CLOSED});
  Tracker t(ks, img);
  auto result = t.readImage();

  GridCurve<KSpace> gc(ks);
  for (int i = 0; i < 2; i++) {
    gc.initFromSCellsVector(result[i]);
    DGtal::trace.info() << "gc: " << gc << std::endl;

    Surfel firstSurfel = *gc.begin();
    Point gcStartPoint = ks.sKCoords(ks.sIndirectIncident(firstSurfel, *ks.sDirs(firstSurfel)));
    DGtal::trace.info() << "gc starts at " << gcStartPoint << std::endl;

    auto codesRange = gc.getCodesRange();
    std::string chain(codesRange.begin(), codesRange.end());
    DGtal::trace.info() << "chain from gc: " << chain << std::endl;

    FreemanChain<KSpace::Integer> fc(chain, gcStartPoint[0], gcStartPoint[1]);
    DGtal::trace.info() << "fc: " << fc << std::endl;
    DGtal::trace.info() << std::endl;

    std::vector<Surfel> surfels;
    surfels.reserve(fc.size());


    // per DGtal::FreemanChain.ih, 0=E(false), 1=N(false), 2=W(t), 3=S(true)
    const std::array<const std::pair<const Point, const bool>, 4> stepInfo = {{
      { Point( 1,  0), false },
      { Point( 0,  1), false },
      { Point(-1,  0), true },
      { Point( 0, -1), true }
    }};
    char char0 = '0';

    Point displacement;
    for (char c : fc.chain) {
      
      auto os = stepInfo.at(c-char0);
      DGtal::trace.info() << c << ": " << os << std::endl;


      Point surfelKCoords = gcStartPoint + displacement + os.first;
      Surfel s = ks.sCell(surfelKCoords, os.second);
      // surfels.push_back(ks.sCell(gcStartPoint + displacement + os.first, os.second));

      DGtal::trace.info() << s << std::endl;

      displacement = displacement + os.first + os.first;
      DGtal::trace.info() << "displacement: " << displacement << std::endl;

    }
    

    // DGtal::trace.info() << "reconstructed: " << fc2gc(ks, fc, gcStartPoint) << std::endl;
    DGtal::trace.info() << std::endl << std::endl;

  }

  return 0;
  /*
   
  // DGtal::trace.info() << result;

  DGtal::GridCurve<KSpace> gc(ks);
  DGtal::Board2D board;
  Point low = img.domain().lowerBound() - Point::diagonal(1);
  Point high = img.domain().upperBound() + Point::diagonal(1);
  board << Domain(low, high);
  
  // DGtal::trace.info() << result.size() << std::endl;

  GradientColorMap<int> cmap_grad(0, (int)result.size());
  cmap_grad.addColor( Color( 50, 50, 255 ) );
  cmap_grad.addColor( Color( 255, 0, 0 ) );
  cmap_grad.addColor( Color( 255, 255, 10 ) );
  cmap_grad.addColor( Color( 25, 255, 255 ) );
  cmap_grad.addColor( Color( 255, 25, 255 ) );
  cmap_grad.addColor( Color( 25, 25, 25 ) );
  

  for (int i = 0; i < result.size(); i++) {
    GridCurve<KSpace> gc(ks);
    gc.initFromSCellsVector(result[i]);
    board << DGtal::CustomStyle(gc.className(), new DGtal::CustomColors(cmap_grad(i), Color::None));   
    board << gc;
  }
  board.saveSVG("arcs.svg");
  return 0;
*/

  //trace.info() << ks << std::endl;


  /*
  trace.info() << PreSurfelFollowers<KPreSpace>::first(surfel, int2ext);
  trace.info() << contour2 << std::endl;
  trace.info() << PreSurfelFollowers<KPreSpace>::unique(surfel, int2ext);
  trace.info() << contour2 << std::endl;
*/

/*
  typedef DGtal::Z2i::Domain Domain;
  typedef ImageSelector<Domain, DGtal::Color>::Type ImageBase;
  typedef SafeConstImage<ImageBase> Image;

  typedef ArcTracker<KSpace, Image> Tracker;
  Image img = DGtal::STBReader<ImageBase>::import("cali2.png");

  trace.info() << img.domain() << std::endl;
  trace.info() << img(img.domain().lowerBound()) << std::endl;
  trace.info() << img(img.domain().lowerBound() - Point(1,1)) << std::endl;

  KSpace ks;
  ks.init(img.domain().lowerBound(), img.domain().upperBound(), {KSpace::Closure::PERIODIC, KSpace::Closure::CLOSED});
  //trace.info() << ks << std::endl;

  //trace.info() << KCA::kCoords(ks.lowerCell()) << KCA::kCoords(ks.upperCell()) << std::endl;
*/
  /*
  Tracker t(ks, img);
  auto result = t.readImage();
  trace.info() << result << std::endl;
  */
  return 0;


 /*
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
  */


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

