#include <Board/Board.h>
#include <Board/Transforms.h>
#include <DGtal/base/BasicFunctors.h>
#include <DGtal/base/BasicTypes.h>
#include <DGtal/base/Trace.h>
#include <DGtal/io/Color.h>
#include <cmath>
#include <cstdint>
#include <deque>
#include <fstream>
#include <ios>
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

#include <DGtal/images/RigidTransformation2D.h>
#include <DGtal/images/ConstImageAdapter.h>
#include <DGtal/io/colormaps/BasicColorToScalarFunctors.h>
#include <boost/concept/assert.hpp>
#include <DGtal/topology/DigitalSetBoundary.h>

using namespace DGtal;
using namespace Z2i;

struct ColorFunctors {
  typedef functors::ColorRGBEncoder<uint32_t> ColorToUInt;
  struct UIntToColor {  
    inline Color operator() (const uint32_t &aRgb) const {
      return Color(aRgb, 255); 
    }
  };

  ColorFunctors() = delete; 
};

struct TransformWebSVG : public LibBoard::TransformSVG {
  double mapWidth( double width ) const {
    return Transform::round(1000 * width) / 1000.0;
  }

  void setBoundingBox(const LibBoard::Rect & rect,
              const double pageWidth,
              const double pageHeight,
              const double margin) {
    _scale = 1;
    _deltaX = 0;
    _deltaY = 0;
    _height = rect.height;
  }

};

class SvgBoard : public Board2D {
 public:
  using Board2D::Board2D;
  SvgBoard(const Board2D& other) : Board2D(other) {}

  void saveWebSVG(const char* filename) {
    auto oldState = Board::State(_state);
    _state = Board::State();
    auto ppmm = 720.0f / 254.0f;
    setUnit(ppmm, LibBoard::Board::UPoint);

    _state.unitFactor = 1;

    std::ofstream file(filename);
    
    TransformWebSVG transform;
    LibBoard::Rect box = boundingBox();
    transform.setBoundingBox(box, box.width, box.height, 0);

    bool clipping = _clippingPath.size() > 2;
    if ( clipping ) box = box && _clippingPath.boundingBox();
    
    file << "<svg version=\"1.1\" xmlns=\"http://www.w3.org/2000/svg\"" << std::endl;
    file << "     width=\"" << ceil(box.width) << "\" "
                  << "height=\"" << ceil(box.height) << "\" "
                  << "preserveAspectRatio=\"xMinYMin meet\"" << std::endl;
    file << "     viewBox=\"" << box.left << " " << (box.height - box.top) << " " << box.width << " " << box.height << "\"" << std::endl;
    file << "     style=\""
                  << "background-color: " << _backgroundColor.svg() << "; "
                  << "display: block; "
                  << "aspect-ratio: " << box.width << " / " << box.height << "; "
                  << "width: auto; "
                  << "max-width: 100%; "
                  << "max-height: 100vh; "
                  << "transform: scale(1,-1);\"";
    file << ">" << std::endl;

    if (clipping) {
      file << "<g clip-rule=\"nonzero\">\n"
                << " <clipPath id=\"GlobalClipPath\">\n"
                << "  <path clip-rule=\"evenodd\"  d=\"";
      _clippingPath.flushSVGCommands( file, transform );
      file << "\" />\n";
      file << " </clipPath>\n";
      file << "<g clip-path=\"url(#GlobalClipPath)\">\n";
    }
    
    // Draw the shapes.
    std::vector< Shape* > shapes = _shapes;
    stable_sort( shapes.begin(), shapes.end(), LibBoard::shapeGreaterDepth );
    std::vector< Shape* >::const_iterator i = shapes.begin();
    std::vector< Shape* >::const_iterator end = shapes.end();
    while ( i != end ) {
      (*i)->flushSVG( file, transform );
      ++i;
    }  

    if (clipping) file << "</g>\n</g>";
    
    file << "</svg>" << std::endl;
    file.close();
    _state = oldState;
  }
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
  uint32_t targetValue = colorToUInt(Color(255, 0, 0));
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

  trace.beginBlock("scanning surfels");
  SurfelAdjacency<2> sAdj(true);
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

  Domain stateDomain(min - Point(1, 1), max + Point(1, 1));

  //ImageFromSet<BoolImage> stateSetImage;
  // DigitalSet::insert()
  
  // DigitalSetDomain<SetOfSurfels<KSpace>> dsd(state);


  SvgBoard stateSurfelBoard;
  
  // stateSurfelBoard << stateDomain;
  stateSurfelBoard << CustomStyle( stateSurfels.begin()->className(),
                         new CustomPenColor(DGtal::Color::None));
  
  for (auto surfel : stateSurfels.surfelSet()) {
    stateSurfelBoard << surfel;
  }
  
  auto mmWidth = 1200 / 3.78;
  RealPoint stateSize = stateDomain.upperBound() - stateDomain.lowerBound();
  auto maxLeg = std::min(stateSize[0], stateSize[1]);
  RealPoint svgSize = mmWidth * stateSize / maxLeg;
  // auto stateHeight = stateDomain.upperBound()[1] - stateDomain.lowerBound()[1];
  // stateSurfelBoard.scale(1, -1);
  //auto stateHeight = stateDomain.upperBound()[1] - stateDomain.lowerBound()[1];
  //trace.info() << stateHeight << std::endl;
  //stateSurfelBoard.translate(0, stateHeight / 2);
  std::string filename = "stateSurfelBoard.svg";
  stateSurfelBoard.saveWebSVG(filename.c_str());
  // writeSvg(stateSurfelBoard, stateDomain, filename);
  // stateSurfelBoard.saveSVG(filename.c_str());
  // stateSurfelBoard.saveSVG("stateSurfelBoard.svg", svgSize[0], svgSize[1]);
/*
  auto cellContours = 
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
  trace.info() << stateSurfels.surfelSet().size();
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
