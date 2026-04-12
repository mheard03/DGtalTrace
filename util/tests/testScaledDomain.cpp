#include <DGtal/base/Trace.h>
#include <DGtal/kernel/PointVector.h>
#include <DGtal/topology/KhalimskyPreSpaceND.h>
#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <iterator>
#include <numeric>
#include <DGtal/base/Assert.h>
#include <DGtal/base/Common.h>
#include <DGtal/helpers/StdDefs.h>
#include <DGtal/topology/KhalimskySpaceND.h>
#include <span>
#include <sstream>
#include <string>
#include <utility>
#include "../DomainScaler.cpp"
#include <ranges>
#include <vector>

template <typename TPoint>
std::vector<DGtal::Dimension> dimIndices() { 
  const DGtal::Dimension dimCount = DGtal::PointVector<2, int>::dimension;
  auto iota = std::views::iota((uint)0, dimCount);
  return std::vector<DGtal::Dimension>(iota.begin(), iota.end());
}

template <typename TPoint>
std::vector<DGtal::PointVector<TPoint::dimension, typename TPoint::Component>> getDimensionVectors() {
  typedef TPoint Point;
  std::vector<Point> dimensions;
  dimensions.resize(TPoint::dimension);
  std::ranges::for_each(dimIndices<TPoint>(), [&](uint dim){ dimensions[dim][dim] = 1; });
  return dimensions;
}

template <typename TPoint>
std::vector<TPoint> getCombinations(std::vector<std::vector<typename TPoint::Component>> dimensionValues) {
  typedef TPoint Point;
  typedef typename Point::Component Component;
  const DGtal::Dimension dims = Point::dimension;
  ASSERT(dimensionValues.size() == dims);

  auto sizes = std::views::transform(dimensionValues, [](auto& v) { return v.size(); });
  int comboCount = std::reduce(sizes.begin(), sizes.end(), 1, std::multiplies<int>{});

  std::vector<Point> combinations;
  combinations.reserve(comboCount);
  combinations.push_back(DGtal::Z2i::Point::zero);

  std::vector<Point> newCombinations;
  newCombinations.reserve(comboCount);

  for (auto dim : dimIndices<Point>()) {
    auto dimValues = dimensionValues[dim];

    // sort followed by unique, to remove all duplicates
    std::sort(dimValues.begin(), dimValues.end());
    auto last = std::unique(dimValues.begin(), dimValues.end());
    dimValues.erase(last, dimValues.end());

    for (Component value : dimValues) {
      Point valuePoint;
      valuePoint[dim] = value;
      for (auto current : combinations) {
        newCombinations.push_back(current + valuePoint);
      }
    }
    combinations.swap(newCombinations);
    newCombinations.clear();
  }

  combinations.shrink_to_fit();
  return combinations;
}

template <typename TPoint>
std::vector<TPoint> getCombinations(std::vector<typename TPoint::Component> repeatedValues) {
  typedef TPoint Point;
  const DGtal::Dimension dims = Point::dimension;
  
  std::vector<std::vector<typename TPoint::Component>> dimensionValues;
  for (uint i = 0; i < dims; i++) {
    dimensionValues.push_back(repeatedValues);
  }
  return getCombinations<TPoint>(dimensionValues);
}

template <typename TPoint>
std::vector<TPoint> getNeighborOffsetVectors(typename TPoint::Component scale = 1, bool includeZero = true) {
  typedef TPoint Point;
  const DGtal::Dimension dims = Point::dimension;
  
  auto neighbors = getCombinations<TPoint>({-scale, 0, scale});
  
  if (!includeZero) {
    auto [first, last] = std::ranges::remove(neighbors, TPoint::zero);
    neighbors.erase(first, last);
  }
  return neighbors;
}

template <typename TPoint>
std::vector<DGtal::PointVector<TPoint::dimension, typename TPoint::Component>> getDirectionVectors() {
  typedef TPoint Point;
  const DGtal::Dimension dims = Point::dimension;
  typedef typename Point::Component Component;

  int pointCount = 1;
  for (int dim = 0; dim < dims; dim++) {
    pointCount *= 2;
  }

  std::vector<int> values;
  for (int i = 0; i < pointCount; i++) {
    values.push_back(i);
  }
  
  std::vector<Point> directions;
  for (int value : values) {
    Point p;
    for (int dim = 0; dim < dims; dim++) {
      p[dim] = value % 2;
      value = value >> 1;
    }
    directions.push_back(p);
  }

  return directions;
}

template <typename TPoint>
std::vector<TPoint> getDomainVertices(TPoint lower, TPoint upper) {
  typedef TPoint Point;
  const DGtal::Dimension dims = Point::dimension;
  typedef typename Point::Component Component;

  std::vector<std::vector<Component>> dimValues(dims);
  for (Point corner : { lower, upper }) {
    for (int dim = 0; dim < dims; dim++) {
      dimValues[dim].push_back(corner[dim]);
    }
  }

  return getCombinations<TPoint>(dimValues);
}

template <typename TDomain, typename TPoint = typename TDomain::Point>
std::vector<std::tuple<TPoint, TPoint, bool>> buildTestPointPairs(int scale, bool scaleUpwards, TDomain& sourceDomain, TPoint targetDomainLower, std::vector<TPoint>& sourcePoints) {
  using namespace DGtal;
  using namespace DGtal::Z2i;

  auto sdScale = (scaleUpwards) ? 1 : scale;
  auto tdScale = (scaleUpwards) ? scale : 1;
  std::vector<std::tuple<Point, Point, bool>> testPoints;
  for (auto offset : { 0, 1, -2 }) {
    Point sp = sourceDomain.lowerBound() + Point::diagonal(offset) * sdScale;
    Point tp = targetDomainLower + Point::diagonal(offset) * tdScale;
    testPoints.push_back({sp, tp, sourceDomain.isInside(sp)});
  }

  return testPoints;
}

template <typename TDomain, typename TPoint = typename TDomain::Point>
std::vector<std::tuple<TPoint, TPoint, bool>> buildTestPointPairs(int scale, bool scaleUpwards, TDomain sourceDomain, TPoint targetDomainLower) {
  using namespace DGtal;
  using namespace DGtal::Z2i;
  typedef typename Point::Container Container;

  auto vertices = getDomainVertices<TPoint>(sourceDomain.lowerBound(), sourceDomain.upperBound());
  auto neighborVectors = getNeighborOffsetVectors<TPoint>((scaleUpwards ? 1 : scale), true);
  
  std::vector<TPoint> sourcePoints;
  for (auto vertex : vertices) {
    for (auto offset : neighborVectors) {
      sourcePoints.push_back(vertex + offset);
    }
  }

  return buildTestPointPairs<TDomain>(scale, scaleUpwards, sourceDomain, targetDomainLower, sourcePoints);
}

int testDomainScalerFunctor() {
  using namespace DGtal;
  using namespace DGtal::Z2i;
  
  trace.beginBlock("Testing DomainScalerFunctor");
  int nbTests = 0, nbPassed = 0;

  // Constructor tests
  {
    Domain smol(Point::diagonal(0), Point::diagonal(10));
    Domain badSize(Point::diagonal(0), Point::diagonal(10) - Point(1,0));

    // tuple: shouldThrow, souceDomain, scale, scaleUpwards, failureText
    std::vector<std::tuple<bool, Domain, int, bool, std::string>> tests = {
      { true, Domain(), 1, false, "Didn't throw when creating a DomainScalerFunctor on an empty sourceDomain." },
      { true, smol, 0, false, "Didn't throw when creating a DomainScalerFunctor with a scale of 0." },
      { false, smol, 1, false, "Threw when creating a valid DomainScalerFunctor" },
      { true, badSize, 5, false, "Didn't throw when creating a DomainScalerFunctor on a wrongly-sized upscaling sourceDomain." },
      { false, badSize, 5, true, "Threw when creating a DomainScalerFunctor on a downscaling sourceDomain." }
    };
    for (auto [shouldThrow, sourceDomain, scale, scaleUpwards, failureText] : tests) {
      try {
        nbTests++;
        auto dsf = DomainScalerFunctor<Domain>(sourceDomain, scale, scaleUpwards);
        if (shouldThrow) {
          trace.info() << failureText << std::endl;
        }
        else {
          nbPassed++;
        }
      }
      catch (const std::invalid_argument& e) {
        if (!shouldThrow) {
          trace.info() << failureText << std::endl;
        }
        else {
          nbPassed++;
        }
      }
    }
  }

  // Main tests
  auto scales = {1, 2, 3};
  int safeDomainSize = 2 * std::reduce(scales.begin(), scales.end(), 1, std::multiplies<int>{});
  for (int scale : scales) {
    for (bool scaleUpwards : { true, false }) {
      auto scaleStr = (scaleUpwards) ? "" : "1/";
      for (Point sourceDomainLower : { Point::diagonal(0), Point::diagonal(1) }) {
        Point sourceDomainUpper = sourceDomainLower + Point::diagonal(safeDomainSize);
        Domain sourceDomain(sourceDomainLower, sourceDomainUpper);
        for (Point targetDomainLower : { Point::diagonal(0), Point::diagonal(2) }) {
          auto testPoints = buildTestPointPairs(scale, scaleUpwards, sourceDomain, targetDomainLower);
          auto dsf = DomainScalerFunctor<Domain>(sourceDomain, scale, scaleUpwards, targetDomainLower);
          for (auto [sourcePoint, expectedTargetPoint, isInside] : testPoints) {
            nbTests++;
            auto calculatedTargetPoint = dsf(sourcePoint);
            if (calculatedTargetPoint == expectedTargetPoint) {
              nbPassed++;
            }
            else {
              trace.info() << "dsf(" << sourcePoint << ") => " << calculatedTargetPoint;
              trace.info() << "Params: scale " << scaleStr << scale << "x, sdl " << sourceDomainLower << ", tdl " << targetDomainLower << ".";
              trace.info() << "Test failed, expected " << expectedTargetPoint << ".";
              trace.info()  << std::endl;
            }            
          }
        }
      }
    }
  }

  trace.emphase() << nbPassed << "/" << nbTests << " tests passed." << std::endl;
  trace.endBlock();
  
  return 1;
}


int testScaledDomainPair() {
  using namespace DGtal;
  using namespace DGtal::Z2i;
  
  trace.beginBlock("Testing ScaledDomainPair");
  int nbTests = 0, nbPassed = 0;


  auto scales = {1, 2, 3};
  int safeDomainSize = std::reduce(scales.begin(), scales.end(), 1, std::multiplies<int>{});
  for (int scale : scales) {
    for (bool scaleUpwards : { true, false }) {
      auto scaleStr = (scaleUpwards) ? "" : "1/";
      for (Point sourceDomainLower : { Point::diagonal(0), Point::diagonal(1) }) {
        Point sourceDomainUpper = sourceDomainLower + Point::diagonal(safeDomainSize);
        for (Point targetDomainLower : { Point::diagonal(0), Point::diagonal(2) }) {
          auto domainPair = ScaledDomainPair<Domain>(sourceDomainLower, sourceDomainUpper, scale, targetDomainLower);

          std::vector<Point> allSourcePoints(domainPair.sourceDomain().begin(), domainPair.sourceDomain().end());
          auto testPoints = buildTestPointPairs(scale, scaleUpwards, domainPair.sourceDomain(), targetDomainLower, allSourcePoints);

          bool passed = true;
          for (auto [sourcePoint, expectedTargetPoint, isInside] : testPoints) {
            auto calculatedTargetPoint = domainPair.toTarget(sourcePoint);
            auto calculatedSourcePoint = domainPair.toSource(calculatedTargetPoint);
            passed &= calculatedTargetPoint == expectedTargetPoint;

            auto spInside = domainPair.sourceDomain().isInside(sourcePoint);
            auto tpInside = domainPair.targetDomain().isInside(calculatedTargetPoint);
            passed &= spInside == tpInside;

            if (!passed) break;
          }

          if (!passed) {
            bool pointPassed = true;
            trace.info() << "Source " << domainPair.sourceDomain() << " x" << scaleStr << scale << " => Target: " << domainPair.targetDomain() << std::endl;
            for (auto [sourcePoint, expectedTargetPoint, isInside] : testPoints) {
              auto calculatedTargetPoint = domainPair.toTarget(sourcePoint);
              auto calculatedSourcePoint = domainPair.toSource(calculatedTargetPoint);
              pointPassed &= calculatedTargetPoint == expectedTargetPoint;

              auto spInside = domainPair.sourceDomain().isInside(sourcePoint);
              auto tpInside = domainPair.targetDomain().isInside(calculatedTargetPoint);
              pointPassed &= spInside == tpInside;

              trace.info() << "  domainPair.toTarget(" << sourcePoint << " & " << spInside << ") => " << calculatedTargetPoint;
              if (!pointPassed) {
                trace.info() << ", expected " << expectedTargetPoint << " & " << tpInside << ".";
              }
              trace.info() << std::endl;
            }
            trace.info() << std::endl;
          }
        }
      }
    }
  }
  /*
  for (int scale : scales) {
    for (bool scaleUpwards : { true, false }) {
      auto scaleStr = (scaleUpwards) ? "" : "1/";
      for (Point sourceDomainLower : { Point::diagonal(0), Point::diagonal(1) }) {
        Point sourceDomainUpper = sourceDomainLower + Point::diagonal(safeDomainSize);
        for (Point targetDomainLower : { Point::diagonal(0), Point::diagonal(2) }) {
          auto domainPair = ScaledDomainPair<Domain>(sourceDomainLower, sourceDomainUpper, scale, targetDomainLower);
          bool dpPassed = true;

          auto testPoints = buildTestPointPairs(scale, scaleUpwards, Domain(sourceDomainLower, sourceDomainUpper), targetDomainLower);
          for (auto [sourcePoint, expectedTargetPoint, isInside] : testPoints) {
            nbTests++;
            bool passed = true;
            
            std::stringstream output("");

            auto calculatedTargetPoint = domainPair.toTarget(sourcePoint);
            auto calculatedSourcePoint = domainPair.toSource(calculatedTargetPoint);
            auto spInside = domainPair.sourceDomain().isInside(sourcePoint);
            auto tpInside = domainPair.targetDomain().isInside(calculatedTargetPoint);

            if (calculatedTargetPoint != expectedTargetPoint) {
              passed = false;
              output << "  domainPair.toTarget(" << sourcePoint << ") => " << calculatedTargetPoint;
              output << ", expected " << expectedTargetPoint << "." << std::endl;
            }
            if (calculatedSourcePoint != sourcePoint) {
              passed = false;
              output << "  domainPair.toSource(" << calculatedTargetPoint << ") => " << calculatedSourcePoint;
              output << ", expected " << sourcePoint << "." << std::endl;
            }
            if (spInside != tpInside) {
              passed = false;
              output << "  calculatedTargetPoint inside: " << domainPair.targetDomain().isInside(calculatedTargetPoint);
              output << ", expected " << domainPair.sourceDomain().isInside(sourcePoint) << "." << std::endl;           
            }

            if (passed) {
              nbPassed++;
            }
            else {
              if (dpPassed && !passed) {
                dpPassed = false;
                trace.info() << "Source " << domainPair.sourceDomain() << " x" << scaleStr << scale << " => Target: " << domainPair.targetDomain() << std::endl;
              }
              trace.info() << output.str() << std::endl;
            }
          }
        }
      }
    }
  }
  */
  trace.endBlock();
}



/*
  template <typename TDomain>
class ScaledDomainPair {
 public:
  // typedef TDomain Domain
  typedef DGtal::Z2i::Domain Domain;
  typedef typename Domain::Point Point;
  typedef DomainScalerFunctor<Domain> Functor;

  ScaledDomainPair(const Point &sourceDomainLower, const Point &sourceDomainUpper, double targetScale, const Point &targetDomainLower = Point::zero) :
    mySourceDomain(sourceDomainLower, sourceDomainUpper),
    myFuncSourceToTarget(Functor(&mySourceDomain, targetScale, false, targetDomainLower)),
    myTargetDomain(myFuncSourceToTarget(sourceDomainLower), myFuncSourceToTarget(sourceDomainUpper)),
    myFuncTargetToSource(Functor(&myTargetDomain, targetScale, true, sourceDomainLower))
  {}

  const Domain& sourceDomain() const { return mySourceDomain; }
  const Domain& targetDomain() const { return myTargetDomain; }
  inline const Point toTarget(Point p) const { return myFuncSourceToTarget(p); }
  inline const Point toSource(Point p) const { return myFuncTargetToSource(p); }

  template <typename TCell> requires CAnyCell<TCell>
  inline const Point toTarget(TCell c) const { return toTarget(c.preCell().coordinates()); }
  template <typename TCell> requires CAnyCell<TCell>
  inline const Point toSource(TCell c) const { return toSource(c.preCell().coordinates()); }

 private:
  const Domain mySourceDomain;
  const DomainScalerFunctor<Domain> myFuncSourceToTarget;
  const Domain myTargetDomain;  
  const DomainScalerFunctor<Domain> myFuncTargetToSource;
};
*/



/*

template <typename TKSpace>
int testCellIterator() {
  typedef TKSpace KSpace;
  typedef typename KSpace::Cell Cell;
  
  KSpace K;
  
  // Closed K-space
  K.init({0, 0}, {5, 5}, true);
  testCellIterator(K);
  Cell lower({5, 5}), upper ({5, 5});
  testCellIterator(K, lower, upper);
  


  // K.init({0, 0}, {20, 20}, { KSpace::Closure::PERIODIC, KSpace::Closure::OPEN });
}
  */