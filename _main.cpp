#include <Board/Board.h>
#include <Board/Transforms.h>
#include <DGtal/base/BasicFunctors.h>
#include <DGtal/base/BasicTypes.h>
#include <DGtal/base/CountedPtr.h>
#include <DGtal/base/Trace.h>
#include <DGtal/dec/Duality.h>
#include <DGtal/geometry/curves/GridCurve.h>
#include <DGtal/geometry/curves/SegmentComputerUtils.h>
#include <DGtal/io/Color.h>
#include <DGtal/kernel/BasicPointPredicates.h>
#include <DGtal/kernel/PointVector.h>
#include <DGtal/kernel/sets/DigitalSetBySTLVector.h>
#include <DGtal/topology/DigitalTopology.h>
#include <DGtal/topology/SCellsFunctors.h>
#include <cmath>
#include <cstdint>
#include <deque>
#include <fstream>
#include <ios>
#include <iterator>
#include <regex>
#include <filesystem>
#include <sstream>
#include <iostream>
#include <sys/types.h>
#include <type_traits>
#include <vector>

#include <DGtal/images/imagesSetsUtils/ImageFromSet.h>
#include <DGtal/kernel/sets/DigitalSetDomain.h>
#include <DGtal/kernel/sets/DigitalSetSelector.h>
#include <DGtal/topology/SurfelAdjacency.h>
#include <iostream>
#include <DGtal/base/Common.h>
#include <DGtal/images/Image.h>
#include <DGtal/images/ImageSelector.h>
#include <DGtal/helpers/StdDefs.h>
#include <DGtal/io/readers/STBReader.h>
#include <string>
#include <unordered_map>
#include <DGtal/geometry/curves/ArithmeticalDSSComputer.h>
#include <DGtal/geometry/curves/SaturatedSegmentation.h>
#include <DGtal/base/FunctorHolder.h>

#include <DGtal/io/boards/Board2D.h>
#include <DGtal/topology/SurfelSetPredicate.h>
#include <DGtal/topology/SetOfSurfels.h>

#include <DGtal/images/ConstImageAdapter.h>
#include <DGtal/io/colormaps/BasicColorToScalarFunctors.h>
#include <DGtal/topology/helpers/BoundaryPredicate.h>
#include <boost/concept/assert.hpp>
#include <DGtal/topology/DigitalSetBoundary.h>
#include "faceUtil.cpp"
#include "traceHelper.cpp"
#include "SvgBoard.cpp"

#include <DGtal/helpers/Shortcuts.h>
#include <DGtal/helpers/ShortcutsGeometry.h>
#include <DGtal/dec/ATSolver2D.h>
#include <DGtal/dec/DiscreteExteriorCalculusFactory.h>
#include <DGtal/geometry/curves/ArithmeticalDSSComputer.h>
#include <DGtal/geometry/curves/estimation/MostCenteredMaximalSegmentEstimator.h>
#include <DGtal/geometry/curves/estimation/SegmentComputerEstimators.h>
#include <DGtal/geometry/surfaces/DigitalSurfaceRegularization.h>
#include <DGtal/io/colormaps/GradientColorMap.h>
#include <DGtal/geometry/curves/SaturatedSegmentation.h>

#include <DGtal/math/linalg/EigenSupport.h>
#include <DGtal/dec/DiscreteExteriorCalculus.h>
#include <DGtal/dec/DiscreteExteriorCalculusSolver.h>
#include <DGtal/dec/DiscreteExteriorCalculusFactory.h>

#include "decExamples/exampleDiscreteExteriorCalculusUsage.cpp"
#include "IIGeometricFunctors2.h"
#include <DGtal/geometry/surfaces/estimation/IIGeometricFunctors.h>
#include <DGtal/geometry/surfaces/estimation/IntegralInvariantCovarianceEstimator.h>
#include <DGtal/topology/ExplicitDigitalSurface.h>

using namespace DGtal;
using namespace Z2i;
typedef Shortcuts<KSpace> SH2;
typedef ShortcutsGeometry<KSpace> SHG2;
auto identity = functors::Identity();
SurfelAdjacency<2> sAdj(true);

struct ColorFunctors {
  typedef functors::ColorRGBEncoder<uint32_t> ColorToUInt;
  struct UIntToColor {  
    inline Color operator() (const uint32_t &aRgb) const {
      return Color(aRgb, 255); 
    }
  };

  ColorFunctors() = delete;  
};

template <typename Value>
struct ValueMatchFunctor {
  ValueMatchFunctor(Value& aValueToMatch) : myValueToMatch(aValueToMatch) {}
  inline bool operator() (const Value &aCol) const {
    return aCol == myValueToMatch;
  }
  private:
    Value myValueToMatch;
};

/*
class ColorNum  {
  typedef DGtal::uint32_t uint;
  uint rgbValue;

 public:
  ColorNum() {}
  ColorNum(const uint rgbValue) : rgbValue(rgbValue) {}
  ColorNum(const Color& color) : rgbValue(getRgbValue(color)) {}
  ColorNum(const ColorNum& other) : rgbValue(getRgbValue(other)) {}

  explicit operator uint() const {
    return rgbValue;
  }
  explicit operator Color() const {
    return Color(rgbValue);
  }

  void selfDisplay(std::ostream &	out) const {
    out << (Color)*this;
  }

 private:
  static uint getRgbValue(const uint value) { return value; }
  static uint getRgbValue(const ColorNum& value) { return value.rgbValue; }
  static uint getRgbValue(const Color& value) { return value.getRGB(); }

  template<typename T>
  struct is_colorLike : std::false_type {};

  template<> struct is_colorLike<ColorNum> : std::true_type {};
  template<> struct is_colorLike<Color> : std::true_type {};
  template<> struct is_colorLike<uint> : std::true_type {};

 public:
  template <typename T, std::enable_if_t<is_colorLike<T>::value, bool> = true>
  bool operator==(const T& other) const { return rgbValue == getRgbValue(other); }

  template <typename T, std::enable_if_t<is_colorLike<T>::value, bool> = true>
  bool operator!=(const T& other) const { return rgbValue != getRgbValue(other); }

  template <typename T, std::enable_if_t<is_colorLike<T>::value, bool> = true>
  bool operator<(const T& other) const { return rgbValue < getRgbValue(other); }

  template <typename T, std::enable_if_t<is_colorLike<T>::value, bool> = true>
  bool operator>(const T& other) const { return rgbValue > getRgbValue(other); }

  template <typename T, std::enable_if_t<is_colorLike<T>::value, bool> = true>
  bool operator<=(const T& other) const { return rgbValue <= getRgbValue(other); }

  template <typename T, std::enable_if_t<is_colorLike<T>::value, bool> = true>
  bool operator>=(const T& other) const { return rgbValue >= getRgbValue(other); }
};

std::ostream& operator<<(std::ostream& os, const ColorNum& obj) {
  obj.selfDisplay(os);
  return os;
}
*/

/*
class ColorNum : private Color {
 typedef DGtal::uint32_t uint;
 typedef Color Base;

 public:
  ColorNum() : Color() {};
  ColorNum(const uint aRgb) : Color(aRgb) {};

  explicit operator uint() const {
    return getRGB();
  }
  explicit operator Color() const {
    return Color((uint)*this);    
  }

  using Base::red;
  using Base::green;
  using Base::blue;
  using Base::setRGBi;
  using Base::selfDisplay;
  using Base::operator==;
  using Base::operator!=;
  using Base::operator<;
  using Base::operator>;
  using Base::operator<=;
  using Base::operator>=;
};
*/
/*
std::vector<std::vector<int>> cross(const std::vector<std::vector<int>>& accum, const std::vector<int>& nextValues) {
  std::vector<std::vector<int>> result;
  if (accum.size() == 0) { 
    result.push_back(std::vector<int>());
    return cross(result, nextValues);
  }
  
  for (auto a : accum) {
    for (auto v : nextValues) {
      std::vector<int> row(a);
      row.push_back(v);
      result.push_back(row);
    }
  }
  return result;
}

void testCross() {
  std::vector<int> nextValues = { 0, 1 };
  std::vector<std::vector<int>> result;

  for (auto i = 0; i < 3; i++) {
    result = cross(result, nextValues);
      for (auto row : result) {
    for (auto v : row) {
      trace.info() << v << ", ";
    }
    trace.info() << std::endl;
  }
  }

}

/*

std::vector<std::vector<int>> cross(const std::vector<std::vector<int>>& accum, const std::vector<int>& nextValues) {
  if (accum.size() == 0) return { nextValues };
  
  std::vector<std::vector<int>> result;
  for (auto a : accum) {
    for (auto v : nextValues) {
      std::vector<int> row(a);
      row.push_back(v);
      result.push_back(row);
    }
  }
  return result;
}

KSpace::Cells uAllFaces(const KSpace& K, const KSpace::Cells& startCells, DGtal::Dimension toDim) {

  DGtal::Dimension fromDim = K.uDim(*startCells.begin());
  if (fromDim > toDim) {
    auto coFaces = K.uCoFaces(startCell);
    while (coFaces.begin())
    if (fromDim + 1 == toDim) return coFaces;

    Cells next;
    for (auto face : coFaces) {
      K.sp
      uAllFaces(K, face, toDim);
    }

  }
}

// TODO: https://stackoverflow.com/a/2156899/875571
KSpace::Cells uAllFaces(const KSpace& K, const Cell& startCell, DGtal::Dimension toDim) {
  KSpace::Cells results;
  DGtal::Dimension fromDim = K.uDim(startCell);

  // Same dimension: We're done
  if (fromDim == toDim) {
    results.push_back(startCell);
    return results;
  }

  if (fromDim > toDim) {
    auto coFaces = K.uCoFaces(startCell);
    while (coFaces.begin())
    if (fromDim + 1 == toDim) return coFaces;

    Cells next;
    for (auto face : coFaces) {
      K.sp
      uAllFaces(K, face, toDim);
    }

  }

  // Just need cofaces
  

  // Just need faces
  if (fromDim - 1 == toDim) return K.uFaces(startCell);


  


  if (fromDim < toDim) {
    auto next = 

  }

  std::vector<Cell> result;
  auto faces = (fromDim > toDim) ? K.uFaces(startCell) : K.uCoFaces(startCell) ;
  for (auto face : faces) {
    if (K.uDim(face) == toDim) result.push_back(face);
  }
  return result;
}

template <typename TIterator>
Domain computeBoundingBox(TIterator begin, TIterator end) {


SCell c;
KSpace K;
K.inc


  Point lower = Domain().upperBound();
  Point upper = Domain().lowerBound();

  TIterator it = begin;
  while (it != end) {
    lower = lower.inf( *it );
    upper = upper.sup( *it );
    ++it;
  }

  return Domain(lower, upper);
}

template <typename T>
struct is_cell : std::false_type {};

template <>
struct is_cell<Cell> : std::true_type {};

template <>
struct is_cell<SCell> : std::true_type {};

*/

template <typename TContainer, std::enable_if_t<std::is_same_v<typename TContainer::value_type, Point>, bool> = true>
Domain getBoundingBox(const KSpace& K, const TContainer& points, int margin = 0) {
  Point lower = K.upperBound();
  Point upper = K.lowerBound();
  for (const Point& point : points) {
    lower = lower.inf(point);
    upper = upper.sup(point);
  }
  if (margin > 0) {
    Point p = Point::diagonal(margin);
    lower -= p;
    upper += p;
  }
  return Domain(lower, upper);
}

template <typename TContainer, std::enable_if_t<std::is_same_v<typename TContainer::value_type, Cell>, bool> = true>
Domain getBoundingBox(const KSpace& K, const TContainer& uCells, int margin = 0) {
  std::unordered_set<Point> points;
  for (const Cell& cell : uCells) {
    for (Cell& spel : uPixelFaces(K, cell)) {
      points.insert(K.uCoords(spel));
    }
  }
  return getBoundingBox(K, points, margin);
}

template <typename TContainer, std::enable_if_t<std::is_same_v<typename TContainer::value_type, SCell>, bool> = true>
Domain getBoundingBox(const KSpace& K, const TContainer& sOrUCells, int margin = 0) {
  std::vector<Cell> uCells;
  uCells.reserve(sOrUCells.size());
  for (SCell cell : sOrUCells) {
    uCells.push_back(K.unsigns(cell));
  }

  return getBoundingBox(K, uCells, margin);
}


typedef ImageSelector<Domain, uint32_t>::Type RgbImage;
typedef ImageSelector<Domain, bool>::Type BoolImage;
typedef uint16_t Key;
typedef ImageSelector<Domain, Key>::Type KeyImage;

ColorFunctors::ColorToUInt colorToUInt;
ColorFunctors::UIntToColor uIntToColor;
/*
template <typename TSourceImage>
struct FloodFiller {
  typedef typename TSourceImage::Domain Domain;
  typedef typename TSourceImage::Point Point;
  typedef typename TSourceImage::Value SourceValue;

  typedef KSpace::Cell Cell;
  typedef KSpace::SCell SCell;
  typedef KSpace::SurfelSet SurfelSet;
  typedef std::unordered_map<Key, SCell> Surfels;
  
  FloodFiller(const TSourceImage& sourceImage, const KSpace& K) : sourceImage(&sourceImage), targetImage(sourceImage.domain()), K(&K) {}

  void run() {
    auto firstSpel = K->uSpel(K->lowerBound());
    auto aSurfel = *K->uLowerIncident(firstSpel).begin();
    auto firstSurfel = K->uFirst(aSurfel);
    auto lastSurfel = K->uLast(aSurfel);


    For each surfel of K-space...
      Get surfel precell
      Get interior & exterior voxel precells
      If either is invalid, continue
      For each sign...
        Get signed surfel
        If surfel is known, continue
        If interiorVoxel has a value on targetImage, continue
        Track boundary
        Add surfels of boundary to set of known surfels
        Fill interior

/
    K->uFaces();

  }

  void run() {
    SurfelSet foundSurfels;
    SurfelSet nextSurfels;
    KSpace KK;
    typedef KSpace::SCell Cell;
  }
/

 private:
  const TSourceImage* sourceImage;
  const KeyImage targetImage;
  const KSpace* K;
  
  inline KSpace::Cells uUpperNeighborhood(const Cell& c) const {
    ASSERT(K->uIsValid(c));
    Cells N;
    for (DGtal::Dimension k = 0; k < KSpace::DIM; ++k ) {
      if (!K->uIsMax( c, k )) N.push_back(K->uGetIncr( c, k ));
    }
    return N;
  }

  inline KSpace::SCells sUpperNeighborhood(const SCell& c) const {
    ASSERT(K->sIsValid(c));
    SCells N;
    for (DGtal::Dimension k = 0; k < KSpace::DIM; ++k ) {
      if (!K->sIsMax( c, k )) N.push_back(K->sGetIncr( c, k ));
    }
    return N;
  }
};

struct KeyImageInfo {
 public:
  typedef std::unordered_map<Key, Color> KeyColors;
  KeyImageInfo(const RgbaImage& sourceImage) : myImage(sourceImage.domain()), myKeyColors() {
    auto domain = myImage.domain();   
    K.init(domain.lowerBound(), domain.upperBound(), true);

    typedef DGtal::Linearizer<Domain, ColMajorStorage> L;
    FloodFiller<RgbaImage, KeyImage, KSpace> filler(sourceImage, K);
    // typedef typename ImageSelector<Domain, bool>::Type PixelStateImage;
    int keyCount = 0;
    for (auto i = 0; i < myImage.size(); i++) {
      if (myImage[i] > 0) continue;
      auto point = L::getPoint(i, domain);
      auto pxColor = sourceImage(point);
      filler.fill(point, myImage, ++keyCount);
      myKeyColors[keyCount] = sourceImage(point);
    }
  }
  const KeyImage& image() const {
    return myImage;
  }
  const KeyColors& keyColors() const {
    return myKeyColors;
  }
  void selfDisplay(std::ostream &	out) const {
    out << "[KeyImageInfo] " << myImage << "; " << myKeyColors.size() << " colors";
  }
 private:
  KSpace K;
  KeyImage myImage;
  KeyColors myKeyColors;
};
std::ostream& operator<<(std::ostream& os, const KeyImageInfo& obj) {
  obj.selfDisplay(os);
  return os;
}
*/


/*
struct ColorNum : private DGtal::Color {
 private:
  typedef DGtal::uint32_t uint;
 
 public:
  ColorNum(const uint aRGBA) {
    setRGBA(aRGBA);
  }
  inline DGtal::Color &
  DGtal::Color::setRGBi( const unsigned char aRedValue,
            const unsigned char aGreenValue,
            const unsigned char aBlueValue,
            const unsigned char aAlphaValue ) {
    myRed = aRedValue;
    myGreen = aGreenValue;
    myBlue = aBlueValue;
    myAlpha = aAlphaValue;
    return *this;
  }


    Color( const Color &aColor ) = default;
  ColorNum() = delete;

  explicit ColorNum(Color& aColor) {
    myValue = fnColorToInt(aColor);
  }
  explicit ColorNum(DGtal::uint32_t aNumber) {
    myValue = aNumber;
  }
  explicit operator DGtal::uint32_t() const {
    return myValue;
  }
 private:
  static functors::ColorRGBEncoder<DGtal::uint32_t> fnColorToInt;
  DGtal::uint32_t myValue;
};

namespace {
  auto identity = functors::Identity();
  auto ColorToInt = functors::ColorRGBEncoder<int>();
  struct IntToColor {
    IntToColor() = delete;
    Color operator()(int rgbValue) {
      return Color()
    }

  }
}

template <typename TImageContainer>
using ColorNumAdapter = ImageAdapter<TImageContainer, typename TImageContainer::Domain, identity, int, functors::Identity, functors::ColorRGBEncoder<int>, Color::getRGBA


typedef ImageAdapter<TImageContainer, TNewDomain, TFunctorD, TNewValue, TFunctorV> Self;
template <typename TImageContainer>
class ColorToIntAdapter : public 
  typedef TNewValue Value;

  BOOST_CONCEPT_ASSERT(( concepts::CImage<TImageContainer> ));
  BOOST_CONCEPT_ASSERT(( concepts::CDomain<TNewValue> ));
  BOOST_CONCEPT_ASSERT(( concepts::CUnaryFunctor<TFunctorV, typename TImageContainer::Value, Value > ));
};

/*
template <typename TImageContainer, typename TNewValue, typename TFunctorV>
class ConstImageValueAdapter {
  typedef ConstImageValueAdapter<TImageContainer, TNewValue, TFunctorV> Self;
  typedef TNewValue Value;

  BOOST_CONCEPT_ASSERT(( concepts::CImage<TImageContainer> ));
  BOOST_CONCEPT_ASSERT(( concepts::CDomain<TNewValue> ));
  BOOST_CONCEPT_ASSERT(( concepts::CUnaryFunctor<TFunctorV, typename TImageContainer::Value, Value > ));
  
  ///Types copied from the container
  typedef TImageContainer ImageContainer;
  typedef DefaultConstImageRange<Self> ConstRange; 
  typedef DefaultImageRange<Self> Range; 
  typedef typename TImageContainer::Value SourceValue;

 public:
    ConstImageValueAdapter(ImageContainer &anImage, ConstAlias<TFunctorV> aFV) : myImagePtr(&anImage), myFV(&aFV) {}
    ConstImageValueAdapter(const ConstImageValueAdapter& other) = default;
    ConstImageValueAdapter& operator=(const ConstImageValueAdapter& other) {
      if (&other != this) {
        myImagePtr = other.myImagePtr;
        myFV = other.myFV;
      }
      return *this;
    }

 protected:
  ImageContainer* myImagePtr;
  const TFunctorV* myFV;
};
*/
/*
// template <class Image>
class FlipYAdapterHelper {
  static functors::Identity identity;
  typedef functors::FlipDomainAxis<RgbImage::Domain> FlipY;
 
 public:
  typedef ImageAdapter<RgbImage, Domain, FlipY, int, functors::Identity, functors::Identity> Type;
  typedef ConstImageAdapter<RgbImage, Domain, FlipY, int, functors::Identity> ConstType;

  FlipYAdapterHelper() = delete;

  static ConstType getConstImageAdapter(ConstAlias<RgbImage> anImage) {
    FlipY flipY(anImage->domain(), {2});
    return ConstType(anImage, anImage->domain(), flipY, identity);
  }

  static Type getImageAdapter(RgbImage& anImage) {
    FlipY flipY(anImage.domain(), {2});
    return Type(anImage, anImage.domain(), flipY, identity, identity);
  }
};
*/
// typedef ConstImageAdapter<RgbaImage, RgbaImage::Domain, 

/*
typedef DGtal::CircleFrom3Points<Point> Circle;
StabbingCircleComputer scc;
scc.doStuff();
double cx = 0, cy = 0, radius = 1;
scc.getSeparatingCircle().getParameters(cx, cy, radius);

*/

/*
template<typename ConstIterator>
std::map<SCell, RealVector> _getNormals(const ConstIterator& itb, const ConstIterator& ite) {
  typedef StandardDSS4Computer<ConstIterator> SegmentComputer;
  typedef TangentFromDSSEstimator<SegmentComputer> TangentEstimator;
  typedef MostCenteredMaximalSegmentEstimator<SegmentComputer, TangentEstimator> MCSEstimator;
  
  SegmentComputer sc;
  TangentEstimator te;
  MCSEstimator mcs(sc, te);
  mcs.init(itb, ite);

  std::map<SCell, RealVector> normals;
  for (auto i = itb; i != ite; ++i) {

  }

  const size_t surfelCount = it.size();

  it.size();
  mcs.eval()

}

std::map<SCell, RealVector> getNormals(const Curve& curve) {
  auto points = curve.getPointsRange();
  if (curve.isClosed())
    return _getNormals(points.c(), points.c()); 
  else 
    return _getNormals(points.begin(), points.end()); 
}
*/

std::vector<GridCurve<KSpace>> getBoundaries(const KSpace& K, const RgbImage& image, const SetOfSurfels<KSpace>& allSurfels) {
  std::vector<GridCurve<KSpace>> result;
  KSpace::SurfelSet searchSet(allSurfels.surfelSet());
  while (searchSet.size() > 0) {
    auto startSurfel = *searchSet.begin();
    auto targetColor = image(K.interiorVoxel(startSurfel));
    
    functors::SCellToInnerPoint<KSpace> functor(K);
    functors::BoundaryPredicate<KSpace, RgbImage> surfelPredicate(K, image, targetColor);    

    std::vector<SCell> contour;
    Surfaces<KSpace>::track2DSurface(contour, K, sAdj, surfelPredicate, startSurfel);
    GridCurve<KSpace> curve;
    curve.initFromSCellsVector(contour);
    result.push_back(curve);
    
    auto oldSize = searchSet.size();
    for (auto surfel : contour) {
      auto num_erased = searchSet.erase(surfel);
      if (num_erased == 0) trace.info() << "ruh roh" << std::endl;
    }
    if (oldSize == searchSet.size()) {
      trace.info() << "fail" << std::endl;
      result.clear();
      return result;
    }
  }
  trace.info() << "traced " << result.size() << " gridCurves" << std::endl;
  return result;
}

void writeSvg(const Board2D& board, const Domain& domain, const std::string filename) {
  RealPoint size = domain.upperBound() - domain.lowerBound();

  // auto stem = std::filesystem::path(filename).stem().string();
  auto tempFilename = "_" + filename;
  board.saveSVG(tempFilename.c_str(), LibBoard::Board::PageSize::BoundingBox, 0);

  // style="display: block; max-width: 100vh; max-height: 100vh; transform: scale(1,-1)"

  // Read all lines into a vector
  std::ifstream inFile(tempFilename);
  if (!inFile.is_open()) {
    std::cerr << "Error opening file for reading: " << tempFilename << std::endl;
    return;
  }

  std::stringstream buffer;
  buffer << inFile.rdbuf();
  auto svgText = buffer.str();
  inFile.close();

  std::regex re("(<svg[^>]*)>", std::regex_constants::icase);
  auto result = std::regex_replace(svgText, re, "$1 style=\"display: block; max-width: 100vh; max-height: 100vh; transform: scale(1,-1)\">", std::regex_constants::format_first_only);

  std::ofstream outFile(filename, std::ios_base::trunc);
  if (!outFile.is_open()) {
    std::cerr << "Error opening file for reading: " << filename << std::endl;
    return;
  }

  outFile << result;
  outFile.flush();
  outFile.close();
  std::remove(tempFilename.c_str());
}

int main(int argc, char** argv) {

  std::string inputFilename = (argc >= 1 ? "cali.png" : argv[1]);

  trace.beginBlock("reading " + inputFilename);
  RgbImage image = STBReader<RgbImage>::import(inputFilename);
  trace.endBlock();
  trace.info() << image.domain() << std::endl;
  trace.info() << image(Point(0,0)) << std::endl;

  auto domain = image.domain();   
  KSpace K;  
  K.init(domain.lowerBound(), domain.upperBound(), true);
  Color targetColor = Color(255, 0, 0);
  uint32_t targetValue = colorToUInt(targetColor);
  auto firstSpel = K.uSpel(K.lowerBound());

  Cell s;

  trace.beginBlock("scanning pixels");
  DigitalSet stateDs(domain);
  s = firstSpel;
  auto lastSpel = K.uSpel(K.upperBound());
  do {
    auto point = K.uCoords(s);
    if (image(point) == targetValue) {
      stateDs.insert(point);
    }
  }
  while (K.uNext(s, firstSpel, lastSpel)); 
  trace.endBlock();

  typedef DiscreteExteriorCalculus<2, 2, EigenLinearAlgebraBackend> Calculus;
  typedef DiscreteExteriorCalculusFactory<EigenLinearAlgebraBackend> CalculusFactory;

  Calculus calculus = CalculusFactory::createFromDigitalSet(stateDs, true);
  trace.info() << calculus << std::endl;
  
  auto derivative = calculus.derivative<1, Duality::DUAL>();
  trace.info() << derivative << std::endl;

  trace.beginBlock("scanning surfels");
  SetOfSurfels<KSpace> stateSurfels(K, sAdj);
  std::vector<bool> tf = { true, false };
  
  Point min(K.upperBound()), max(K.lowerBound());
  KSpace::PreCellularGridSpace PK;
  for (auto aSurfel : K.uLowerIncident(firstSpel)) {
    s = K.uFirst(aSurfel);
    do {
      for (auto sign : tf) {
        auto ss = K.signs(s, sign);

        auto pss = ss.preCell();
        auto pInside = PK.interiorVoxel(pss);
        auto pOutside = PK.exteriorVoxel(pss);
        if (!domain.isInside(pInside) || !domain.isInside(pOutside)) continue;

        if (image(pInside) == targetValue && image(pOutside) != targetValue) {
          for (auto i = 0; i < K.DIM; i++) {
            min[i] = std::min(min[i], pInside[i]);
            max[i] = std::max(max[i], pInside[i]);
          }
          stateSurfels.surfelSet().insert(ss);
          continue;
        }
      }
    }
    while ( K.uNext( s, K.uFirst(aSurfel), K.uLast(aSurfel) ) );
  }
  trace.endBlock();

  trace.beginBlock("getting boundaries");
  auto boundaries = getBoundaries(K, image, stateSurfels);
  for (auto b : boundaries) {
    trace.info() << "  closed: " << b.isClosed() << "; valid: " << b.isValid() << std::endl;
  }
  trace.endBlock();

  /*

  auto h = 0.01;

  typedef GridCurve<KSpace>::IncidentPointsRange Range;
  typedef Range::ConstIterator ClassicIterator;
  typedef Range::ConstCirculator CircularIterator;
  auto gridcurve = boundaries[0];
  Range r = gridcurve.getIncidentPointsRange(); //building range
  // Estimation
  std::vector<double> estimations;
  {
    //! [MostCenteredConstruction]
    typedef StabbingCircleComputer<ClassicIterator> ScComputer;
    ScComputer sc;

    sc.
    typedef CurvatureFromDCAEstimator<SegmentComputer> SCEstimator;
    typedef MostCenteredMaximalSegmentEstimator<SegmentComputer,SCEstimator> CurvatureEstimator;
    
    SCEstimator sce;
    CurvatureEstimator estimator(sc, sce);
    //! [MostCenteredConstruction]

    std::cout << "# open grid curve" << endl;
    
    //! [MostCenteredInit]
    estimator.init( r.begin(), r.end() );
    //! [MostCenteredInit]
    
    //! [MostCenteredEvaluation]
    estimator.eval( r.begin(), r.end(), std::back_inserter(estimations), h );
    //! [MostCenteredEvaluation]

          std::cout << "# idx kappa" << endl;
      unsigned int i = 0;
      for ( ClassicIterator it = r.begin(), ite = r.end();
           it != ite; ++it, ++i )
        {
          std::cout << i << " " << estimations.at(i) << std::endl;
        }
  }
*/


  Domain stateDomain(min - Point(1, 1), max + Point(1, 1));
  trace.info() << "stateDomain: " << stateDomain << std::endl;
  Domain stateDomain2 = getBoundingBox(K, stateDs.container());
  trace.info() << "stateDomain2: " << stateDomain2 << std::endl;
  
  Domain stateDomain3 = getBoundingBox(K, stateSurfels.surfelSet(), 10);
  trace.info() << "stateDomain3: " << stateDomain3 << std::endl;


  trace.beginBlock("getting normals");
  typedef DigitalSurface<SetOfSurfels<KSpace>> DigitalSurfaceFromSurfels;
  typedef functors::IINormalDirectionFunctor<Space> NormalFunctor;
	typedef IntegralInvariantCovarianceEstimator<KSpace, DigitalSurfaceFromSurfels, NormalFunctor> IINormalEstimator;
  
  DigitalSurfaceFromSurfels surface(stateSurfels);


  typedef functors::BoundaryPredicate<KSpace, RgbImage> BoundaryPredicate;
  BoundaryPredicate bp(K, image, targetValue);
  

  // stateSurfels.surfelSet();

  const double radiusII = 6;
  const double h = 1;
	NormalFunctor normalFunctor;
	IINormalEstimator normalEstimator( normalFunctor );
  /*
	normalEstimator.attach(K, surface);	
	normalEstimator.setParams(radiusII / h);
  auto ss = stateSurfels.surfelSet();
	normalEstimator.init( h, ss.begin(), ss.end());

  for (auto surfel : ss) {
    trace.info() << "surfel: " << surfel << "; norm: " << normalEstimator.eval(&surfel).getNormalized() << std::endl;
	}
	trace.endBlock();
*/

/*
  typedef ConstImageAdapter<RgbImage, RgbImage::Domain, functors::Identity, bool, ValueMatchFunctor<DGtal::uint32_t>> BinaryImageAdapter;

  auto cmf = ValueMatchFunctor(targetValue);
  BinaryImageAdapter bImage(image, domain, identity, cmf);
  // KSpace::SurfelSet allSurfels;
  // Surfaces<KSpace>::sMakeBoundary(allSurfels, K, bImage, K.lowerBound(), K.upperBound());

  auto params    = SH2::defaultParameters();
  params["surfelAdjacency"] = 1;

  typedef DigitalSurface<SetOfSurfels<KSpace>> DS;
  auto stateSurfacePtr = CountedPtr<DS>(new DS(stateSurfels));
  auto surfels = SH2::getSurfelRange(stateSurfacePtr, params );
  // auto linels = SH2::getCellRange(stateSurfacePtr, 1 );
  
  typedef functors::IINormalDirectionFunctor<Space> IINormalFunctor;
  typedef IntegralInvariantCovarianceEstimator<KSpace, BinaryImageAdapter, IINormalFunctor> IINormalEstimator;
  typedef std::vector<RealVector> RealVectors;
  typedef RealVector::Component Scalar;

  RealVectors n_trivial;
  n_trivial.reserve(surfels.size());
  for (auto s : surfels) {
    Dimension k = K.sOrthDir( s );
    bool direct = K.sDirect( s, k );
    RealVector t = RealVector::zero;
    t[k] = direct ? -1.0 : 1.0;
    n_trivial.push_back( t );
  }

  RealVectors n_estimations;
  Scalar h = 1.0;       // gridstep
  Scalar r = 3.0;       // r-radius
  Scalar alpha = 0.33;

  if ( alpha != 1.0 ) r *= pow( h, alpha-1.0 );

  IINormalFunctor functor;
  functor.init(h, r*h);
  IINormalEstimator ii_estimator(functor);
  ii_estimator.attach(K, bImage);
  ii_estimator.setParams(r);
  ii_estimator.init(h, surfels.begin(), surfels.end() );

  trace.info() << "surfel:" << *(surfels.begin()) << std::endl;
  ii_estimator.eval(surfels.begin()); // , surfels.end(), std::back_inserter(n_estimations));
  ii_estimator.eval(surfels.begin(), surfels.end(), std::back_inserter(n_estimations));
  trace.info() << "ii_estimate:" << ii_estimator.eval(surfels.begin()) << std::endl;

  // orientVectors
  /*
  std::transform( n_trivial.cbegin(), n_trivial.cend(), n_estimations.cbegin(), n_estimations.begin(),
                [] ( RealVector rw, RealVector w )
                { return rw.dot( w ) >= 0.0 ? w : -w; } );


                /*

  GradientColorMap<size_t> cm(0, boundaries.size() - 1);
  cm.addColor(Color::Red);
  cm.addColor(Color(255, 165, 0));
  cm.addColor(Color::Green);
  cm.addColor(Color::Blue);

  SvgBoard stateSurfelBoard;
  // stateSurfelBoard << stateDomain3;
  for (auto i = 0; i < boundaries.size(); i++) {
    auto b = boundaries[i];
    for (auto s : b) {
      stateSurfelBoard << CustomStyle(s.className(), new CustomColors(Color::None, cm(i)));
      stateSurfelBoard << s;
    }
  }
  std::string filename = "stateSurfelBoard.svg";
  stateSurfelBoard.saveWebSVG(filename.c_str());

  /*
  // stateSurfelBoard << stateDomain;
  stateSurfelBoard << CustomStyle( stateSurfels.begin()->className(),
                         new CustomPenColor(DGtal::Color::None));
  for (auto surfel : stateSurfels.surfelSet()) {
    stateSurfelBoard << surfel;
  }
  
  std::string filename = "stateSurfelBoard.svg";
  stateSurfelBoard.saveWebSVG(filename.c_str());

  /*
  std::vector<std::vector<SCell>> subshapes;
  Surfaces<KSpace>::extractAllConnectedSCell(subshapes,K, sAdj, stateDs, false);

  std::vector<DigitalSet> subsets;
  for (auto cells : subshapes) {
    stateDs.computeBoundingBox(Point &lower, Point &upper)
    DigitalSet subset();

    subsets.push_back(DigitalSet())
  }
  std::set<Curve> gridCurves;
  
  while (remainingSurfels.size()) {
    DigitalSet ds(stateDomain);
    DigitalSetBoundary<KSpace, DigitalSet> dsb(K, ds);

    
    auto surfel = *remainingSurfels.begin();
    auto tracker = stateSurfels.newTracker(surfel);
    std::deque<SCell> boundarySurfels = { surfel };
    auto next = tracker->move(const Surfel &s)

    
    Surfaces<KSpace>::tratrack2DBoundaryPoints( boundaryPoints, ks, sAdj, dig, bel );

    DigitalSetBoundary<KSpace, KSpace::SurfelSet> dsb()

    Curve c;
    c.initFromSCellsRange(const TIterator &itb, const TIterator &ite)
  }
  stateSurfels.
  Curve c;
  c.
*/
  // trace.info() << stateSurfels.surfelSet().size();
/*

  BoolImage interiorImage(domain);
  typedef DGtal::functors::SurfelSetPredicate<std::set<SCell>, SCell> ssp;
  unsigned int nbInt = Surfaces<KSpace>::uFillInterior(K, ssp(state.surfelSet()),
                                                                                   interiorImage, 1, false);
  trace.info() << "Interior size: " << nbInt << " (awaited:  3082)" <<  std::endl;
  */
  /*
  trace.beginBlock("generating keyImage");
  auto keyImage = KeyImageInfo(image);
  trace.endBlock();
  trace.info() << keyImage << std::endl;
  return 0;
  */
}
