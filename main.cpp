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
#include <DGtal/kernel/domains/Linearizer.h>
#include <algorithm>
#include <array>
#include <cmath>
#include <exception>
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

//#include "CEVec.cpp"
//#include "PixelNeighborhoods/CWAnglesEnum.hpp"
//#include "PixelNeighborhoods/DirectionsOffsets.cpp"
//#include "DGtal/geometry/curves/BinomialConvolver.h"

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

//#include "GraphTracer/tests/testCellIterator.cpp"
//#include "GraphTracer/MultiCellIterator.cpp"
//#include "util/tests/testScaledDomain.cpp"

#include <algorithm>
#include <cmath>
#include <iostream>
#include <iterator>
#include <list>
#include <ranges>
#include <sstream>
#include <string>
#include <type_traits>
#include <set>
#include <map>
#include <type_traits>
#include <vector>
//#include "faceUtil.cpp"
//#include "traceHelper.cpp"
//#include "SvgBoard.cpp"
//#include "IIGeometricFunctors2.h"
//#include "KSpaceLinearizer.cpp"
// #include "PixelNeighborhood.cpp"

#include "util/KDomain.cpp"
#include <boost/polygon/polygon.hpp>

using namespace DGtal;

/*
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
// typedef Z2i::KSpace KSpace;

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

template <typename T>
std::vector<std::vector<T>> getCombinations(std::vector<std::vector<T>> inDimensionValues) {
  std::vector<std::vector<T>> dimensionValues;
  for (auto dimValues : inDimensionValues) {
    std::vector<T> uniques;
    std::ranges::sort(dimValues);
    std::ranges::unique_copy(dimValues, std::back_inserter(uniques));
    dimensionValues.push_back(uniques);
  }

  std::vector<std::vector<T>> combinations;
  std::vector<std::vector<T>> newCombinations;
  combinations.push_back({});

  for (auto dimValues : dimensionValues) {
    for (T value : dimValues) {
      for (std::vector<T> current : combinations) {
        std::vector<T> currentPlus(current);
        currentPlus.push_back(value);
        newCombinations.push_back(currentPlus);
      }
    }
    combinations.swap(newCombinations);
    newCombinations.clear();
  }

  combinations.shrink_to_fit();
  return combinations;
}

template <typename T>
std::vector<std::vector<T>> getCombinations(std::vector<T> repeatedValues, uint repeatCount) {
  std::vector<std::vector<T>> dimensionValues;
  for (uint i = 0; i < repeatCount; i++) {
    dimensionValues.push_back(repeatedValues);
  }
  return getCombinations<T>(dimensionValues);
}

using namespace DGtal::Z2i;
typedef typename std::pair<Point, Point> Region;

constexpr std::optional<Region> intersect(const Region& rect1, const Region& rect2) {
  typedef typename Region::first_type Point;

  Region result;
  for (DGtal::Dimension k = 0; k < Point::dimension; ++k) {
    result.first[k] = std::max(rect1.first[k], rect2.first[k]);
    result.second[k] = std::min(rect1.second[k], rect2.second[k]);
    if (result.first[k] >= result.second[k]) return std::nullopt; // disjoint
  }
  return result;
}

template <typename Compare>
constexpr bool compare(const Point& p1, const Point& p2, Compare comparator) {
  for (DGtal::Dimension k = 0; k < Point::dimension; ++k) {
    if (comparator(p1[k], p2[k])) return false;
  }
  return true;
}
constexpr bool contains(const Region& r1, const Region& r2) {
  return compare(r1.first, r2.first, std::less_equal<>()) &&
         compare(r1.second, r2.second, std::greater_equal<>());
}
/*
template <DGtal::Dimension dimension>
constexpr std::vector<Region> splitRegions(DGtal::ConstAlias<KSpace> ks, const DGtalTrace::KDomain<KSpace>& domain) {
  Region domainRegion = { 
    domain.lowerBound().preCell().coordinates, 
    domain.upperBound().preCell().coordinates
  };
  Region kRegion = { 
    ks->lowerCell().preCell().coordinates, 
    ks->upperCell().preCell().coordinates 
  };
  trace.info() << "splitRegions (" << kRegion << ", " << domainRegion << ")" << std::endl;

  if (!ks->isAnyDimensionPeriodic()) return { domainRegion };
  {
    // if the domain wraps all the way around the space, ensure the start and end match the space start and end
    // avoids doubling the number of sub-domains
    auto normSecond = ks->uCell(domainRegion.second).preCell().coordinates;
    for (DGtal::Dimension k = 0; k < KSpace::dimension; k++) {
      if (!ks->isSpacePeriodic(k)) continue;
      if (normSecond[k] == domainRegion.second[k]) continue;
      if (normSecond[k] + 1 == domainRegion.first[k]) {
        domainRegion.first[k] = kRegion.first[k];
        domainRegion.second[k] = kRegion.second[k];
      }
    }
  }
  if (contains(kRegion, domainRegion)) return { domainRegion };

  trace.info() << "continuing splitRegions (" << kRegion << ", " << domainRegion << ")" << std::endl;
  std::array<std::vector<int>, KSpace::dimension> periodicity = {};
  periodicity.fill({0});
  for (DGtal::Dimension k = 0; k < KSpace::dimension; k++) {
    if (ks->isSpacePeriodic(k)) periodicity[k].push_back(1);
  }

  std::vector<Region> regions;
  Vector kExtent  = kRegion.second - kRegion.first + Point::diagonal(1);
  trace.info() << "regionExtent " << kExtent << std::endl;
  for (auto const& tuple : std::apply(std::views::cartesian_product, periodicity)) {
    Vector v = std::make_from_tuple<Vector>(tuple);
    Region offsetRegion = { 
      domainRegion.first - v * kExtent, 
      domainRegion.second - v * kExtent
    };
    trace.info() << "v" << v << ", offset " << (v * kExtent) << ", offsetRegion" << offsetRegion << std::endl;
    auto intersection = intersect(kRegion, offsetRegion);
    if (intersection) {
      regions.push_back(*intersection);
    }
  }


  for (auto region : regions) {
    trace.info() << region.first << "->" << region.second << "; ";
  }
  trace.info() << std::endl;
  return regions;
}
  */
  /*
  print(tuple, i++);
[&v]<size_t... Is>(std::index_sequence<Is...>)
        { return std::views::cartesian_product((Is, v)...); }
        (std::make_index_sequence<N>{});


  for (DGtal::Dimension k = 0; k < Point::dimension; ++k) {
    if (!ks->isSpacePeriodic(k)) continue;

    std::vector<Region> newRegions;
    for (const Region& r : regions) {
      auto base = intersect(r, kBounds);
      if (maybeSplit) {
        newRegions.push_back(*maybeSplit);
      }
      else {
        Region split1 = r;
        split1.second[k] = kBounds.first[k];
        newRegions.push_back(split1);

        Region split2 = r;
        split2.first[k] = kBounds.second[k] + 1;
        newRegions.push_back(split2);
      }
    }
    regions.swap(newRegions);
  }
  typedef DGtalTrace::KDomain<KSpace> KDomain;
  const Point kLower = ks->lowerCell().preCell().coordinates;
  const Point kUpper = ks->upperCell().preCell().coordinates;

  std::vector<Region> rects = {{ domainLowerCell.preCell().coordinates, domainUpperCell.preCell().coordinates }};

  for (DGtal::Dimension k = 0; k < Point::dimension; k++) {
    if (!ks->isSpacePeriodic(k)) continue;
    std::vector<Region> newRects;
    for (auto rect : rects) {
      auto& [srcLower, srcUpper] = rect;
      if (srcUpper[k] <= kUpper[k]) {
trace.info() << "k" << k << " srcUpper " << srcUpper << " < kUpper " << kUpper << std::endl;
      }
      
      // First block: rect.lower to right edge of domain
      {
        Point firstUpper = srcUpper;
        firstUpper[k] = kUpper[k];
        newRects.push_back({ srcLower, firstUpper });
      }

      // Second block: right edge of domain to rect.upper
      {
        Point secondLower = srcUpper;
        secondLower[k] = kUpper[k] + 1;
        // secondLower = KDomain::normalizeCell(ks, secondLower).preCell().coordinates;
        // Point secondUpper = KDomain::normalizeCell(ks, srcUpper).preCell().coordinates;
        newRects.push_back({ secondLower, srcUpper });
      }
    }
    rects.clear();
    newRects.swap(rects);
    trace.info() << "rects.size():" << rects.size() << "newRects.size():" << newRects.size() << std::endl;
  }
  return rects;
}
template <size_t N>
auto product(auto&& v)
{
    return [&v]<size_t... Is>(std::index_sequence<Is...>)
        { return std::views::cartesian_product((Is, v)...); }
        (std::make_index_sequence<N>{});
}
*/

int main(int argc, char** argv)
{
  using namespace DGtal::Z2i;

  std::vector<KSpace::Closure> closureValues = {
    KSpace::Closure::CLOSED,
    KSpace::Closure::OPEN,
    KSpace::Closure::PERIODIC
  };
  auto closureCombinations = getCombinations(closureValues, KSpace::dimension);
  typedef typename std::array<KSpace::Closure, KSpace::dimension> ClosureArray;

  Point p1 = Point::diagonal(0);
  Point p2 = Point::diagonal(4);
  std::vector<KSpace> allSpaces;
  for (auto closureSet : closureCombinations) {
    KSpace k;
    ClosureArray closure;
    for (auto i = 0; i < KSpace::dimension; i++) {
      closure[i] = closureSet[i];
    }
    k.init(p1, p2, closure);
    allSpaces.push_back(k);
  }

  std::vector<KSpace> testSpaces;
  {
    bool isNonPeriodicFound = false;
    for (auto ks : allSpaces) {
      if (ks.isAnyDimensionPeriodic() || !isNonPeriodicFound) {
        testSpaces.push_back(ks);
        isNonPeriodicFound = true;
      }
    }
  }


  using namespace Z2i;

  for (auto ks : testSpaces) {
    std::stringstream ss;
    ss << ks;
    trace.beginBlock(ss.str());

    std::map<std::string, std::pair<Cell, PreCell>> pairs;
    {
      auto lowerBound = ks.uKCoords(ks.lowerCell());
      auto upperBound = ks.uKCoords(ks.upperCell());
      auto kSize = upperBound - lowerBound + Point::diagonal(1);
      auto periodicDimDiagonal = Point::diagonal(0);
      int periodicDim = -1;
      for (auto k = 0; k < KSpace::dimension; k++) {
        if (ks.isSpacePeriodic(k)) {
          periodicDimDiagonal[k] = 1;
          periodicDim = k;
        }
      }

      std::map<std::string, std::pair<Point, Point>> pointPairs;
      pointPairs["basic"] = {{2,2}, {4,4}};
      pointPairs["narrow"] = {{2,2}, {2,4}};
      pointPairs["full"] = {lowerBound, upperBound};
      if (ks.isAnyDimensionPeriodic()) {
        pointPairs["wide"] = {{3,3}, Point(4,4) + (kSize - 3) * periodicDimDiagonal };
        pointPairs["periodicFull"] = {{2,2}, {4,4}};
        for (auto k = 0; k < KSpace::dimension; k++) {
          if (ks.isSpacePeriodic(k)) {
            pointPairs["periodicFull"].first[k] = lowerBound[k];
            pointPairs["periodicFull"].second[k] = upperBound[k];
          }
        }
      }

      for (auto [key, pointPair] : pointPairs) {
        auto [lowerPoint, upperPoint] = pointPair;
        pairs.insert_or_assign(key, std::pair{ ks.uCell(lowerPoint), KPreSpace::uCell(upperPoint) });
      }
    } // pairs population

    typedef DGtalTrace::KDomain<KSpace> KDomain;

    // bool writtenOne = false;
    for (auto [key, cellPair] : pairs) {
      trace.beginBlock(key);
      auto [lower, upper] = cellPair;
      
      KDomain d(ks, lower, upper);
      trace.info() << ks << std::endl;
      trace.info() << d << std::endl;


      trace.endBlock(); // key
    }
    trace.endBlock();   // kspace
    trace.info() << std::endl;
  }
    
  return 0;
}
    /*
    
    trace.beginBlock("Upper");
    trace.beginBlock("normalizePoint");
    {
      auto lower = KPreSpace::uCell(commonLower);
      auto upper = KPreSpace::uCell(basicUpper);

    }
      for (auto preCell : { first, second }) {
        auto cell = K.uCell(preCell);
        auto normCell = K.uCell(KDomain::normalizePoint(K, preCell.coordinates));
        
        auto dim = PKSpace::uDim(cell);
        auto normDim = PKSpace::uDim(normCell);
        if (cell != normCell || dim != normDim) {
          trace.info() << preCell << "-> cell " << cell << ", " << " normCell " << normCell << std::endl;
        }
      }
    }
    trace.endBlock();



    
    wide, inverted, offset = upper;
    for (int k = 0; k < KSpace::dimension; k++) {
      points.normal = std::pair{P(defaultFirst), P(defaultSecond)};
      points.narrow = std::pair{P(defaultFirst), P(defaultSecond)};
      points.wide = std::pair{P(defaultFirst), P(defaultSecond)};
      points.inverted = std::pair{P(defaultFirst), P(defaultSecond)};
      points.offset = std::pair{P(defaultFirst), P(defaultSecond)};
    }
    for (int k = 0; k < Z2i::Point::dimension; k++) {
      auto kWidth = ks.upperCell().preCell().coordinates[k] - ks.lowerCell().preCell().coordinates[k];
      if (ks.isSpacePeriodic(k)) {
        points.narrow.second.coordinates[k] = points.narrow.first.coordinates[k];
        points.wide.second.coordinates[k] += 1.5 * kWidth;
        points.inverted.second.coordinates[k] -= kWidth;
        points.offset.first.coordinates[k] -= kWidth;
        points.offset.second.coordinates[k] -= kWidth;
      }
    }
    return points;
  }

  typedef DGtalTrace::KDomain<Z2i::KSpace> KDomain;
  typedef Z2i::KSpace KSpace;
  typedef Z2i::KPreSpace PKSpace;
  typedef PKSpace::Cell PreCell;
  for (auto const K : spaces) {
    if (!K.isAnyDimensionPeriodic()) continue;
    std::stringstream ss;
    ss << K;
    trace.beginBlock(ss.str());
    CellSet cellSet = getCells(K);
    auto cells = { cellSet.normal, cellSet.narrow, cellSet.wide, cellSet.inverted, cellSet.offset };

    trace.beginBlock("normalizePoint tests");
    for (auto [first, second] : cells) {
      for (auto preCell : { first, second }) {
        auto cell = K.uCell(preCell);
        auto normCell = K.uCell(KDomain::normalizePoint(K, preCell.coordinates));
        
        auto dim = PKSpace::uDim(cell);
        auto normDim = PKSpace::uDim(normCell);
        if (cell != normCell || dim != normDim) {
          trace.info() << preCell << "-> cell " << cell << ", " << " normCell " << normCell << std::endl;
        }
      }
    }
    trace.endBlock();

    trace.beginBlock("normalizeUpperPoint tests");
    // cells = { cellSet.normal, cellSet.narrow, cellSet.wide, cellSet.inverted, cellSet.offset };
    cells = { cellSet.wide };
    for (auto [first, second] : cells) {
      auto lower = KDomain::normalizePoint(K, first.coordinates);
      auto lowerCell = K.uCell(lower);

      auto upper = KDomain::normalizeUpperBound(K, first.coordinates, second.coordinates);
      auto upperPreCell = Z2i::KPreSpace::Cell(upper);
      auto upperCell = K.uCell(upper);

      auto oldOffset = second.coordinates - first.coordinates;
      auto newOffset = upper - lower;

      auto dim = PKSpace::uDim(PKSpace::uCell(upper));
      auto normDim = PKSpace::uDim(upperPreCell);

      // if (oldOffset != newOffset || dim != normDim) {
        trace.info() << "Lower: " << first.coordinates << " -> " << lower << " -> " << lowerCell << std::endl;
        trace.info() << "Upper: " << second.coordinates << " -> " << upper << " -> " << upperPreCell << " -> " << upperCell << std::endl;
        trace.info() << "Offset:" << oldOffset << " -> " << newOffset << std::endl;
        trace.info() << "Dimension: " << dim << "->" << normDim << std::endl;
      // }
    }    
    trace.endBlock();

    trace.endBlock();
  }
  */
/*
  auto allSurfels = KPreSpace::uLowerIncident(KPreSpace::uSpel({0,0}));
  std::vector<KPreSpace::Cell> uniqueSurfels;
  std::ranges::unique_copy(allSurfels, std::back_inserter(uniqueSurfels), {}, [](auto& c) { return KPreSpace::uTopology(c); });
  std::sort(uniqueSurfels.begin(), uniqueSurfels.end());
  trace.info() << "allFaces: " << allSurfels << std::endl;
  trace.info() << "uniqueFaces: " << uniqueSurfels << std::endl;

  for (auto K : spaces) {
    std::stringstream ss;
    ss << K;
    trace.beginBlock(ss.str());

    for (auto surfel : uniqueSurfels) {
      Domain d(K.uFirst(surfel).preCell().coordinates, K.uLast(surfel).preCell().coordinates);
      trace.info() << "Surfel: " << surfel << " (dim " << K.uDim(K.uFirst(surfel)) << ") -> " << d << " (size " << d.size() << ")" << std::endl;
    }
    trace.endBlock();
  }
*/

  

  
  /*
  typedef DGtal::Z2i::Point Point;
  trace.info() << "[2,2] * [1,3] = " << (Point({2,2}) * Point({1,3})) << std::endl;

  Point p(11, 17);
  trace.info() << "Before: " << p << std::endl;
  p[0] = 100;
  trace.info() << "After: " << p << std::endl;

  auto combos2 = getDirectionVectors<DGtal::Z2i::Point>();
  trace.info() << "Z2i: " << combos2 << std::endl;;

  auto combos3 = getDirectionVectors<DGtal::Z3i::Point>();
  trace.info() << "Z3i: " << combos3 << std::endl;;

  std::vector<Point> dv = getDimensionVectors<Point>();
  trace.info() << "Z2i: " << dv << std::endl;

  auto combinations = getCombinations<Point>({{1,2,3}, {10, 11, 12, 13}});
  trace.info() << "getCombinations: " << combinations << std::endl;

  auto combinations2 = getCombinations<Point>({-1, 0, 0, 1});
  trace.info() << "getCombinations2: " << combinations2 << std::endl;

  auto neighbors = getNeighborOffsetVectors<Point>();
  trace.info() << "getNeighborOffsetVectors() " << neighbors << std::endl;
  neighbors = getNeighborOffsetVectors<Point>(2);
  trace.info() << "getNeighborOffsetVectors(2) " << neighbors << std::endl;
  neighbors = getNeighborOffsetVectors<Point>(2, false);
  trace.info() << "getNeighborOffsetVectors(2, false) " << neighbors << std::endl;


  auto vertexPoints = getDomainVertices(Point(1,1), Point(3,5));
  trace.info() << "getDomainVertices: " << vertexPoints << std::endl;

  testDomainScalerFunctor();
  testScaledDomainPair();
  return 0;

  /*
  KSpace K;
  K.init({0, 0}, {15, 15}, { KSpace::Closure::PERIODIC, KSpace::Closure::OPEN });

  auto cell = K.uSpel({1, 1});
  trace.info() << "cell: " << cell << std::endl;
  for (auto face : K.uFaces(cell)) {
    trace.info() << "face: " << face << " (dim " << K.uDim(face) << ", top " << K.uTopology(face) << ")" << std::endl;
  }
  return 0;
  /*
  testCellIterator<KSpace::Cell>(K, {0, 0});
  testCellIterator<KSpace::Cell>(K, {-1, 0});
  testCellIterator<KSpace::Cell>(K, {0, 1});


  return 0;
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
  // KSpace K;  
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
