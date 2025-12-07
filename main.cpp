/**
  * This program is free software : you can redistribuate it and/or modify
  * it under the terms of the GNU Lesser General Public License as
  * published by the Free Software Foundation, either version 3 of the
  * License, or (at you option) any later version.
  *
  * This program is distributed int the hope that it will be useful,
  * but WITHOUT ANY WARRANTY; without even the implied warranty of
  * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  * GNU General Public License for more details.
  *
  * You should have received a copy of the GNU General Public License
  * along with this program. If not, set <http://www.gnu.org/license/>.
 **/

/// @file dec/exampleHeatLaplace.cpp
/**
   Example of the Heat Laplace Operator.
   @see \ref secHowToHeatLaplace
   \example dec/exampleHeatLaplace.cpp
**/

////////// INCLUDES /////////

#include <DGtal/base/BasicFunctors.h>
#include <DGtal/kernel/PointVector.h>
#include <algorithm>
#include <array>
#include <cmath>
#include <functional>
#include <DGtal/helpers/StdDefs.h>

#include <Board/Rect.h>
#include <Board/Shapes.h>
#include <DGtal/base/Circulator.h>
#include <DGtal/io/boards/Board2D.h>
#include <DGtal/shapes/fromPoints/StraightLineFrom2Points.h>

#include <DGtal/base/Common.h>
#include <DGtal/base/ConstAlias.h>
#include <DGtal/base/ConstRangeAdapter.h>
#include <DGtal/images/Image.h>
#include <DGtal/images/ImageSelector.h>
#include <DGtal/images/ConstImageAdapter.h>
#include <DGtal/io/readers/STBReader.h>

#include <DGtal/kernel/BasicPointPredicates.h>
#include <DGtal/kernel/PointFunctorHolder.h>
#include <DGtal/topology/DigitalSurface.h>
#include <DGtal/topology/DigitalSetBoundary.h>
#include <DGtal/topology/SetOfSurfels.h>
#include <DGtal/topology/LightImplicitDigitalSurface.h>

#include <DGtal/math/linalg/EigenSupport.h>

#include <DGtal/dec/DiscreteExteriorCalculus.h>
#include <DGtal/dec/DiscreteExteriorCalculusFactory.h>
#include <DGtal/dec/DiscreteExteriorCalculusSolver.h>
#include <DGtal/dec/VectorField.h>

#include <DGtal/geometry/surfaces/estimation/LocalEstimatorFromSurfelFunctorAdapter.h>
#include <DGtal/geometry/surfaces/estimation/IIGeometricFunctors.h>
#include <DGtal/geometry/surfaces/estimation/IntegralInvariantCovarianceEstimator.h>

#include "CEVec.cpp"
#include "PixelNeighborhoods/CWAnglesEnum.hpp"
#include "PixelNeighborhoods/DirectionsOffsets.cpp"
#include "DGtal/geometry/curves/BinomialConvolver.h"

#include <DGtal/io/readers/GenericReader.h>
#include <DGtal/io/colormaps/ColorBrightnessColorMap.h>
#include <DGtal/io/colormaps/TickedColorMap.h>
#include "DGtal/io/readers/GenericReader.h"

#include <DGtal/images/IntervalForegroundPredicate.h>
#include <DGtal/images/imagesSetsUtils/SetFromImage.h>

#include <DGtal/shapes/parametric/Ball2D.h>
#include <DGtal/shapes/Shapes.h>
#include <DGtal/shapes/GaussDigitizer.h>

#include <DGtal/geometry/curves/ArithmeticalDSSComputer.h>
#include <DGtal/geometry/curves/SaturatedSegmentation.h>
#include <DGtal/geometry/curves/StabbingCircleComputer.h>
#include <DGtal/geometry/curves/StabbingLineComputer.h>
#include <DGtal/topology/CanonicSCellEmbedder.h>
#include "PixelNeighborhoods/PixelRelativePointFunctor.cpp"



#include <set>
#include <map>
#include <type_traits>
#include <vector>
#include "faceUtil.cpp"
#include "traceHelper.cpp"
#include "SvgBoard.cpp"
#include "IIGeometricFunctors2.h"
#include "KSpaceLinearizer.cpp"
// #include "PixelNeighborhood.cpp"

using namespace DGtal;
using namespace Z2i;

struct Options
{
  double h;
  double normal_radius;
  double convolution_radius;
};

struct ColorFunctors {
  typedef functors::ColorRGBEncoder<uint32_t> ColorToUInt;
  struct UIntToColor {  
    inline Color operator() (const uint32_t &aRgb) const {
      return Color(aRgb, 255); 
    }
  };

  ColorFunctors() = delete;  
};
ColorFunctors::ColorToUInt colorToUInt;
ColorFunctors::UIntToColor uIntToColor;

typedef ImageSelector<Domain, uint32_t>::Type RgbImage;
typedef Z2i::KSpace KSpace;

template <typename Image>
struct ValueMatchFunctor {
  typedef typename Image::Point Point;
  typedef typename Image::Value Value;

  ValueMatchFunctor(ConstAlias<Image> anImage, const Value& aValueToMatch) : myImagePtr(&anImage), myValueToMatch(aValueToMatch) {}
  inline bool operator() (const Value &aValue) const {
    return aValue == myValueToMatch;
  }
  private:
    const Image* myImagePtr;
    Value myValueToMatch;
};

const std::vector<Vector> cornerCellDirs = {{1, 1}, {1, -1}, {-1, -1}, {-1, 1}};

struct CornerPoints {
  Point anchorPixel;
  std::array<Point, 2> interiorPoints;
  std::array<Point, 5> exteriorPoints;
};

inline Point pointFromSpelAndOffset(const PreCell& spel, const Vector& offset) {
  return KPreSpace::uCoords(KPreSpace::uTranslation(spel, offset));
}

/*
std::vector<NeighborSample> getSamples(const Cell& pointel, const KSpace& K, const RgbImage& image) {
  typedef KPreSpace PK;
  constexpr std::array<CEVec2, 4> offsets = {CEVec2(1, -1), CEVec2(1, 1), CEVec2(-1, 1), CEVec2(-1, -1) };
  const CEVec2 pointelKCoords = PK::uKCoords(pointel);

#define TRY_READ(enum, number) \
NeighborSample sample##number; \
const CEVec2 centerPoint##number = PK::uCoords(PK::uSpel(pointelKCoords + offsets[(int)enum])); \
if (NeighborSample::tryRead<enum>(sample##number, centerPoint##number, K, image)) result.push_back(sample##number);

  std::vector<NeighborSample> result;
  FOR_ANGLES(TRY_READ)
  return result;

#undef TRY_READ
}
*/
/*
bool isCorner2(const Cell& pointel, const KSpace& K, const RgbImage& image) {
  typedef KPreSpace PK;
  
  std::vector<NeighborSample> samples;
  auto pointelKCoords = K.uKCoords(pointel);
  auto spelNW = PK::uCell(CEVec2(1, 1) + pointelKCoords);
  if (K.uIsInside(spelNW)) {
    uIsInside())
  }
  for (auto pixel : uPixelFaces(K, pointel)) {
    auto offset = K.u
  }
  if (pixels.size() != 4) {
    return false;
  }

  std::vector<std::string> dirNames = {"NE", "SE", "SW", "NW"};
  std::vector<Vector> dirs = cornerCellDirs;
}
*/
/*
bool isMaybeCorner(const Cell& pointel, const KSpace& K, const RgbImage& image) {
  auto pixels = uPixelFaces(K, pointel);
  if (pixels.size() != 4) {
    return false;
  }

  std::vector<std::string> dirNames = {"NE", "SE", "SW", "NW"};
  std::vector<Vector> dirs = cornerCellDirs;
  
  std::vector<Cell> cells(4);
  std::vector<Point> points(4);
  std::vector<RgbImage::Value> colors(4);

  std::transform(dirs.begin(), dirs.end(), cells.begin(), [&](auto& v) { return K.uCell(K.uKCoords(pointel) + v); });
  std::transform(cells.begin(), cells.end(), points.begin(), [&](auto& c) { return K.uCoords(c); });
  std::transform(points.begin(), points.end(), colors.begin(), [&](auto& p) { return image(p); });

  const uint NE = colors[0];
  const uint SE = colors[1];
  const uint SW = colors[2];
  const uint NW = colors[3];
  
  if (NE == SE && NE == SW && NE == NW) return false; // single color
  if (NE == SE && NW == SW) return false; // vertical stripes
  if (NE == NW && SE == SW) return false; // horizontal stripes
  if ((NE == SW) != (NW != SE)) return false; // checkerboard

  
  for (auto i = 0; i < dirNames.size(); i++) {
    trace.info() << pointel << " + " << dirNames[i] << " |"
                 << " dir: " << dirs[i]
                 << ", cell: " << cells[i]
                 << ", point: " << points[i]
                 << ", color: " << colors[i] << std::endl;
  }
  
  return true;
}

const KPreSpace PK;

bool isInside4(const Cell& pointel, const KSpace& K) {
  return uPixelFaces(K, pointel).size() == 4;
}

Vector getQuadrantVector(const Cell& pointel, const Cell& spel, const KSpace& K) {
  for (auto candidatePreCell : uPixelFaces(PK, pointel.preCell())) {
    if (K.uCell(PK.uKCoords(candidatePreCell)) == spel) {
      return PK.uKCoords(candidatePreCell) - K.uKCoords(pointel);
    }
  }
  return { 0, 0 };
}

CornerPoints getCornerCells(const PreCell& pointel, const Vector& quadrantVector) {
  typedef KPreSpace PK;
  auto dX = Vector(quadrantVector[0], 0);
  auto dY = Vector(0, quadrantVector[1]);
  std::array<Vector, 2> interiorDirs = { dX, dY }; // E, N
  std::array<Vector, 5> exteriorDirs = { dY-dX, -dX, -dY-dX, -dY, -dY+dX }; // NW, W, SW, S, E

  auto anchorCell = PK::uCell(PK::uKCoords(pointel) + quadrantVector);
  
  CornerPoints result;
  result.anchorPixel = PK::uCoords(anchorCell);
  std::transform(interiorDirs.begin(), interiorDirs.end(), 
                 result.interiorPoints.begin(), 
                 [&](auto& o) { return pointFromSpelAndOffset(anchorCell, o); });
  
  std::transform(exteriorDirs.begin(), exteriorDirs.end(), 
                 result.exteriorPoints.begin(), 
                 [&](auto& o) { return pointFromSpelAndOffset(anchorCell, o); });

  return result;
}

bool isCorner(const Cell& pointel, const KSpace& K, const RgbImage& image) {
  // if (!isMaybeCorner(pointel, K, image)) return false;

  for (auto dir : cornerCellDirs) {
    auto cornerPoints = getCornerCells(pointel, dir);
    if (!image.domain().isInside(cornerPoints.anchorPixel)) continue;
    auto anchorColor = image(cornerPoints.anchorPixel);
    if (anchorColor == 0) continue;
    if (std::any_of(cornerPoints.interiorPoints.begin(), cornerPoints.interiorPoints.end(), [&](auto& p) { return image.domain().isInside(p) && image(p) != anchorColor; })) continue; // TODO: all_of, since only one of the two interior points needs to be the right color
    if (std::any_of(cornerPoints.exteriorPoints.begin(), cornerPoints.exteriorPoints.end(), [&](auto& p) { return image.domain().isInside(p) && image(p) == anchorColor; })) continue;
    return true;
  }
  return false;

}
  */
/*
template <int Angle>
void testPointOffsetter(const Point& point) {
  PointOffsetter<Angle> offsetter;
  trace.beginBlock("testPointOffsetter<" + std::to_string(Angle) + ">");
  trace.info() << "Point: " << point << std::endl;
  trace.info() << offsetter.NW(point) << std::endl;
  trace.endBlock();
}

template <int Angle, typename TCell, typename TKSpace = typename TCell::CellularGridSpace>
void testCellOffsetter(const TCell& cell) {
  PreCellOffsetter<TCell, Angle> offsetter;
  trace.beginBlock("testCellOffsetter<" + std::to_string(Angle) + ">");
  trace.info() << "Cell: " << cell << std::endl;
  trace.info() << offsetter.all(cell) << std::endl;
  trace.endBlock();
}
*/

int main(int argc, char** argv)
{
  // constexpr std::array<int, 2> x = {1, 2};
  constexpr CEVec2 q1(0, 1);
  constexpr CEVec2 q2(2, -4);
  trace.info() << "q1: " << q1 << std::endl;
  trace.info() << "q2: " << q2 << std::endl;
  trace.info() << "sum: " << (q1+q2) << std::endl;
  
  KSpace::Vector v = q1;
  trace.info() << "v: " << v << std::endl;


  std::string inputFilename = (argc >= 1 ? "cali.png" : argv[1]);
  trace.beginBlock("reading " + inputFilename);
  RgbImage image = STBReader<RgbImage>::import(inputFilename);
  trace.endBlock();

  std::function<double(const RealPoint&, const RealPoint&)> l2_distance =
      [](const RealPoint& a, const RealPoint& b) { return (a - b).norm(); };

  trace.beginBlock("Extracting Digital Surface");

  auto domain = image.domain();   
  KSpace K;  
  K.init(domain.lowerBound(), domain.upperBound(), { KSpace::Closure::PERIODIC, KSpace::Closure::OPEN });

  typedef DGtalTrace::Offsets::CWAngles Angles;
  auto dirArrays = DGtalTrace::Offsets::Impl::makeDirArrays();
  
  trace.info() << dirArrays << std::endl;

  typedef functors::Identity Identity;
  typedef DGtalTrace::Offsets::PixelRelativePointFunctor<RgbImage> PRP;
  typedef ConstImageAdapter<RgbImage, RgbImage::Domain, PRP, RgbImage::Value, Identity> PRPAdapter;

  
  PRP fnPrp(K);
  constexpr Identity fnIdentity;
  PRPAdapter prp(image, image.domain(), fnPrp, fnIdentity);
  trace.info() << prp << std::endl;

  std::vector<Angles> angles = { Angles::Rotate0, Angles::Rotate90, Angles::Rotate180, Angles::Rotate270 };
  std::vector<Point> centers = { { 0, 0}, {-10, 0}, {0, -1}};
  std::vector<Point> offsets = { { 0, 0}, {-1, 0}, {0, -1}};
  for (auto c : centers) {
    fnPrp.setCenterPoint(c);
    for (auto a : angles) {
      fnPrp.setAngle(a);
      trace.info() << "Center: " << c << ", CWAngle: " << a << std::endl;
      for (auto p : offsets) {
        
        trace.info() << "  Offset: " << p << "; Coords: " << fnPrp(p) << "; isInside: " << fnPrp.isInside(p) << std::endl;
      }
    }
  }

  
/*
  typedef NeighborSampler<Angles::Rotate0> SB;
  std::vector<Point> centers = { {50, 50}, {0, 10}, {116, 15}, {0, 0} };
  std::vector<Point> offsets = { {0, 0}, {-1, 0} };

  // {-2, -2}, {1, 1}, 
  for (auto c : centers) {
    SB s(c, image, K);
    trace.info() << "angle: " << ((int)SB::cwAngle * 90) << "; c: " << c << "; bounds: " << s.bounds << std::endl;
    if (s.isValid()) {
      trace.info() << "  NW coords: " << s.nwCoords() << "; value: " << s.nw() << std::endl;
      trace.info() << "  S  coords: " << s.sCoords() << "; value: " << s.s() << std::endl;
    }
    else {
      trace.info() << "  image coords out of bounds";
    }
  }
  */
  /*

  Point point = {2, 2};
  testPointOffsetter<90>(point);

  {
    KSpace K;
    K.init({0, 0}, {20, 20}, true);
    Cell c = K.uCell(point);
    testCellOffsetter<90>(c.preCell());
  }

  {
    Point NW(-2, -1);
    Point SE(1, 2);

    trace.info() << "getRotatedBounds<" << 0 << ">: " << getRotatedBounds<0>(NW, SE) << std::endl;
    trace.info() << "getRotatedBounds<" << 90 << ">: " << getRotatedBounds<90>(NW, SE) << std::endl;
    trace.info() << "getRotatedBounds<" << 180 << ">: " << getRotatedBounds<180>(NW, SE) << std::endl;
    trace.info() << "getRotatedBounds<" << 270 << ">: " << getRotatedBounds<270>(NW, SE) << std::endl;
  }

  {
    Point source(10, 10);
    Point NW(-2, -1);
    Point SE(1, 2);
    PointOffsetter<0> PO0;
    PointOffsetter<90> PO90;
    PointOffsetter<180> PO180;
    PointOffsetter<270> PO270;
    
    trace.info() << "getRotatedBounds<" << 0 << ">: " << PO0.getRotatedBounds(source, NW, SE) << std::endl;
    trace.info() << "getRotatedBounds<" << 90 << ">: " << PO90.getRotatedBounds(source, NW, SE) << std::endl;
    trace.info() << "getRotatedBounds<" << 180 << ">: " << PO180.getRotatedBounds(source, NW, SE) << std::endl;
    trace.info() << "getRotatedBounds<" << 270 << ">: " << PO270.getRotatedBounds(source, NW, SE) << std::endl;
  }
*/
  return 0;
/*
  Options options;

  options.h = 0.1;
  options.normal_radius = 2.0;
  options.convolution_radius = 0.1;


  getTopologicallyUnique(K);
  
  trace.info() << std::endl << std::endl;

  KBounds kb(K.uSpel({10, 10}), K, std::array<uint, 2>({10, 10}));

  auto midY = domain.upperBound()[1] / 2;
  Cell first = K.uSpel(domain.upperBound() + Point({-domain.upperBound()[0]-5, -midY-5}));
  Cell last =  K.uSpel(domain.upperBound() + Point({+5, -midY+5}));

  trace.info() << first << " -> " << last << std::endl;

  Cell p = first;
  do {
    trace.info() << p << std::endl;
  }
  while ( K.uNext( p, first, last ) );

  return 0;

  auto pixel = K.uSpel({10, 10});
  auto surfel = *K.uLowerIncident(pixel).begin();
  auto pointel = *K.uLowerIncident(surfel).begin();

  for (auto cell : { pixel, surfel, pointel }) {
    trace.info() << "Dim: " << K.uDim(cell) << "; Dirs: ";
    for ( auto q = K.uDirs(cell); q != 0; ++q ) {
      const DGtal::Dimension k = *q;
      trace.info() << k << ", ";
    }
    trace.info() << "; OrthDirs: ";
    for ( auto q = K.uOrthDirs(cell); q != 0; ++q ) {
      const DGtal::Dimension k = *q;
      trace.info() << k << ", ";
    }
    trace.info() << std::endl;
  }

  auto pixels = uPixelFaces(K, pointel);
  trace.info() << "cells: " << pixels << std::endl;
  auto tu_cells = getTopologicallyUnique(K, pixels);
  trace.info() << "tu_cells: " << tu_cells << std::endl;


  
  for (int i = 0; i < 3; i++) {
    trace.info() << "toDim: " << i << std::endl;
    trace.info() << "pointel: " << pointel << "distances: " << getDistanceToEdges(K, pointel, i) << std::endl;
    trace.info() << "surfel: " << surfel << "distances: " << getDistanceToEdges(K, surfel, i) << std::endl;
    trace.info() << "pixel: " << pixel << "distances: " << getDistanceToEdges(K, pixel, i) << std::endl;
    trace.info() << std::endl;
  }
  return 0;

  auto firstPointel = K.uFirst(K.uPointel({0, 0}));
  Cell c = firstPointel;
  std::vector<Cell> corners;
  do {
    if (isCorner(c, K, image)) {
      corners.push_back(c);
    }
  }
  while ( K.uNext(c, K.uFirst(c), K.uLast(c)) );

  trace.info() << "corners: " << corners << std::endl;


  Color targetColor = Color(255, 0, 0);
  uint32_t targetValue = colorToUInt(targetColor);
  auto firstSpel = K.uSpel(K.lowerBound());

  std::set<Curve> gridCurves;


  typedef ValueMatchFunctor<RgbImage> VMF;
  VMF vmf(image, targetValue);
  functors::PointFunctorPredicate<RgbImage, VMF> pfp(image, vmf);

  
  trace.beginBlock("extracting surface");
  SurfelAdjacency<KSpace::DIM> surfAdj(true); 
  std::vector<std::vector<SCell>> stateContours;
  Surfaces<KSpace>::extractAll2DSCellContours(stateContours, K, surfAdj, pfp);
  
  SvgBoard board;
  for (auto contour : stateContours) {
    for (auto surfel : contour) {
      board << K.interiorVoxel(surfel);
    }
  }

  for (auto pointel : corners) {
    board << pointel;
  }
  
  board.saveWebSVG("stateSurfelBoard.svg");
  return 0;

  // todo: ConstCirculator?
  // typedef Curve::IncidentPointsRange CurvePointsRange;
  // typedef StabbingLineComputer<CurvePointsRange::ConstIterator> SegmentComputer;
  typedef std::vector<Point> CurvePointsRange;
  typedef StandardDSS4Computer<CurvePointsRange::const_iterator> SegmentComputer;
  typedef SaturatedSegmentation<SegmentComputer> Segmentation;
  SegmentComputer segmentComputer;

  typedef BinomialConvolver<Curve::MidPointsRange::ConstIterator, double> Convolver;
  typedef TangentFromBinomialConvolverFunctor<Convolver, RealPoint> TangentBCFct;
  BinomialConvolverEstimator<Convolver, TangentBCFct> tangentEstimator;

  
  
  // board.drawImage(inputFilename, 0, 0, bbox.width, bbox.height, bbox.width, bbox.height);
  auto embedder = CanonicSCellEmbedder<KSpace>(K);
  auto pointelFromSurfel = [&K, &embedder](SCell surfel, bool reverse = false) { 
    Dimension track = *(K.sDirs(surfel));
    if (reverse) K.sSetSign(surfel, K.sSign(surfel));
    auto pointel = K.sIndirectIncident(surfel, track);
    // K.sSetKCoords(pointel, K.sKCoords(pointel) + Space::Point::diagonal());
    // return pointel;
    auto realPoint = embedder.embed(pointel);
    auto roundedPoint = Space::Point(realPoint + Space::RealPoint::diagonal(0.5));
    return roundedPoint;
  };

  int i = 0;
  for (auto &sc : stateContours) {
    Curve c(K);
    c.initFromSCellsVector(sc);
    board << c;

    std::vector<Point> pointels;
    pointels.reserve(sc.size() + (c.isOpen() ? 1 : 0));
    if (c.isOpen()) pointels.push_back(pointelFromSurfel(*c.begin(), true));
    for (auto surfel : c) {
      pointels.push_back(pointelFromSurfel(surfel));
    }
    trace.info() << "Surfels: " << c << std::endl;
    trace.info() << "Pointels: " << pointels << std::endl << std::endl;
    
    auto pointsRange = pointels;
    Segmentation segmentation(pointsRange.begin(), pointsRange.end(), segmentComputer);

    for (auto s : segmentation) {
      auto dss = s.primitive();
      board << SetMode(dss.className(), "BoundingBox");
      board << CustomStyle(dss.className(), new CustomPenColor(Color::Blue));
      board << dss;
      */
      /*
      // board << s;
      auto hull = s.getPreimage()->pHull();
      auto pLine = StraightLineFrom2Points<Point>(*hull.begin(), *(++hull.begin()));
      hull = s.getPreimage()->qHull();
      auto qLine = StraightLineFrom2Points<Point>(*hull.begin(), *(++hull.begin()));

      std::vector<Point> points = { pLine.p(), pLine.q(), qLine.p(), qLine.q() };
      auto zeroPoint = std::find(points.begin(), points.end(), Point());
      if (zeroPoint == points.end()) continue;

      board << CustomStyle(pLine.className(), new CustomPenColor(Color::Red)) << pLine;
      board << CustomStyle(qLine.className(), new CustomPenColor(Color::Blue)) << qLine;
      */
    /*
    typedef SegmentComputer::Primitive DSS;
    for (auto s : segmentation) {

      DSS longDss = s.primitive(); 
      auto oldFront = longDss.front();
      auto oldBack = longDss.back();

      DSS shortDss(longDss);
      shortDss.retractBack();
      shortDss.retractFront();
      if (shortDss.front() == shortDss.back()) continue; */
      /*
shortDss.
      auto length = (shortDss.front() - shortDss.back()).norm();
      if (length > 30) {
        shortDss.
        trace.info() << "Point: " << p << "; remainder: " << dss.remainder(p) << std::endl;
      }
      */
      // board << SetMode(longDss.className(), "BoundingBox");
      // board << CustomStyle(longDss.className() + "/BoundingBox", new CustomPenColor(Color::Red));
      // board << longDss;
      /*
      board << SetMode(shortDss.className(), "BoundingBox");
      board << CustomStyle(shortDss.className(), new CustomPenColor(Color::Blue));
      board << shortDss;
      */
      /*



      bool isLongBoi = false;
      if (!longBoiFound) {
        auto length = (dss.front() - dss.back()).norm();
        if (length < 30) continue;
        longBoiFound = true;
        // isLongBoi = true;
        for (auto p : dss) {
          trace.info() << "Point: " << p << "; remainder: " << dss.remainder(p) << std::endl;
        }
      }
      auto penColor = board.penColor();
      board << SetMode(dss.className(), "BoundingBox");
      if (isLongBoi) board.setPenColorRGBi(0, 0, 255);
      board << dss; 
      if (isLongBoi) board.setPenColor(penColor);
      
    } 
  
continue;

    auto surfelMidpointsRange = c.getMidPointsRange();
    trace.info() << i++ << ". Closed: " << c.isClosed() << "; curve: " << c << std::endl;
    
    std::vector<RealPoint> tangents(surfelMidpointsRange.size()); 

    trace.info() << "Suggested size: " << Convolver::suggestedSize(1.0, surfelMidpointsRange.begin(), surfelMidpointsRange.end()) << std::endl;
    tangentEstimator.init(0, surfelMidpointsRange.begin(), surfelMidpointsRange.end(), c.isClosed());
    tangentEstimator.eval(surfelMidpointsRange.begin(), surfelMidpointsRange.end(), tangents.begin());
    
    std::vector<RealPoint> surfelMidpoints(surfelMidpointsRange.begin(), surfelMidpointsRange.end());
    double one = 1;
    for (int j = 0; j < tangents.size(); j++) {
      auto midpoint = surfelMidpoints[j];
      auto tangent = tangents[j];
      auto endpoint = midpoint + tangent;
      board.drawArrow(midpoint[0], midpoint[1], endpoint[0], endpoint[1]);
      auto fractPart = std::abs(std::modf(midpoint[0] + midpoint[1], &one));
      if (fractPart != 0 && fractPart != 0.5) {
        trace.info() << "  midpoint: " << midpoint << "; tangent: " << tangent << "; endpoint: " << endpoint << std::endl;
      }
    }
  }

  board.saveWebSVG("stateSurfelBoard.svg");
*/
  return 0;
}
