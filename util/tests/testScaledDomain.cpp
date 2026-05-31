#include <numeric>
#include <DGtal/base/Assert.h>
#include <DGtal/base/Common.h>
#include <DGtal/topology/KhalimskySpaceND.h>
#include "../DomainScaler.cpp"

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
        trace.info() << failureText << std::endl;
      }
      catch (const std::invalid_argument& e) {
        nbPassed++;
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
        Domain sd(sourceDomainLower, sourceDomainUpper);
        for (Point targetDomainLower : { Point::diagonal(0), Point::diagonal(2) }) {
          auto dsf = DomainScalerFunctor<Domain>(sd, scale, scaleUpwards, targetDomainLower);
          auto sdScale = (scaleUpwards) ? 1 : scale;
          auto tdScale = (scaleUpwards) ? scale : 1;
          std::vector<std::tuple<Point, Point>> testPairs;
          for (auto offset : { 0, 1, -2 }) {
            Point sp = sourceDomainLower + Point::diagonal(offset) * sdScale;
            Point tp = targetDomainLower + Point::diagonal(offset) * tdScale;
            testPairs.push_back({sp, tp});
          }
          for (auto [sourcePoint, expectedTargetPoint] : testPairs)
          {
            nbTests++;
            auto calculatedTargetPoint = dsf(sourcePoint);
            trace.info() << "dsf(" << sourcePoint << ") => " << calculatedTargetPoint;
            trace.info() << "Params: scale " << scaleStr << scale << "x, sdl " << sourceDomainLower << ", tdl " << targetDomainLower << ".";

            if (calculatedTargetPoint == expectedTargetPoint) {
              nbPassed++;
            }
            else {
              trace.info() << "Test failed, expected " << expectedTargetPoint << ".";
            }
            trace.info()  << std::endl;
          }
        }
      }
    }
  }

  trace.emphase() << nbPassed << "/" << nbTests << " tests passed." << std::endl;
  trace.endBlock();
  
  return 1;
}
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