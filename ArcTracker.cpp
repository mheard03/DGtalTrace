#pragma once
#include <boost/range/adaptor/transformed.hpp>
#include <iostream>
#include <ranges>
#include <unordered_set>
#include <boost/range/adaptors.hpp>
#include <DGtal/base/Assert.h>
#include <DGtal/base/Trace.h>

#include <DGtal/base/Common.h>
#include <DGtal/io/Color.h>
#include <DGtal/images/ImageSelector.h>
#include <DGtal/topology/SurfelAdjacency.h>
#include <DGtal/topology/helpers/FrontierPredicate.h>
#include <DGtal/topology/helpers/Surfaces.h>
#include <DGtal/topology/KhalimskyCellHashFunctions.h>
#include <DGtal/geometry/curves/GridCurve.h>
#include "concepts/CXCellularGridSpaceND.cpp"
#include "concepts/dgtal/images/CConstImage.cpp"


namespace DGtalTrace {

template <concepts::CXCellularGridSpaceND TKSpace, concepts::CConstImage TImage>
requires (TKSpace::dimension == 2)
class ArcTracker {
  typedef DGtal::SurfelAdjacency<TKSpace::dimension> Adjacency;
  static inline const Adjacency sAdj = Adjacency(false);
 public:
  typedef TImage Image;
  typedef Image::Value Value;
  typedef typename Image::Domain Domain;
  typedef TKSpace KSpace;
  typedef typename KSpace::PreCellularGridSpace KPreSpace;
  typedef typename KSpace::Point Point;
  typedef typename KSpace::SCell Surfel;
  typedef typename DGtal::functors::FrontierPredicate<KSpace, Image> Predicate;
  typedef typename TImage::Value ImageValue;

  static KSpace getExpanded(const KSpace ks) {
    typedef typename KSpace::Closure Closure;

    Point lowerBound = ks.lowerBound();
    Point upperBound = ks.upperBound();
    std::array<Closure, KSpace::dimension> dimClosures;
    for (auto k = 0; k < KSpace::dimension; k++) {
      Closure c = dimClosures[k] = ks.getClosure(k);
      if (c == Closure::CLOSED) {
        lowerBound[k] -= 1;
        upperBound[k] += 1;
      }
    }

    KSpace result;
    result.init(lowerBound, upperBound, dimClosures);
    return result;
  }

  std::vector<Surfel> trackSurface(const Surfel aSurfel) {
    std::vector<Surfel> result;
    auto predicate = Predicate(ksExpanded, myImgPtr,
                               myImgPtr->operator()(ksExpanded.interiorVoxel(aSurfel)),
                               myImgPtr->operator()(ksExpanded.exteriorVoxel(aSurfel)));
    DGtal::Surfaces<KSpace>::track2DSurface(result, ksExpanded, sAdj, predicate, aSurfel);
    return result;
  }

  ArcTracker() {}

  ArcTracker(DGtal::ConstAlias<KSpace> aSpace, DGtal::ConstAlias<Image> anImage) :
    myKPtr(&aSpace), ksExpanded(getExpanded(aSpace)), myImgPtr(&anImage) {}

  std::vector<std::vector<Surfel>> readImage() {
    std::vector<std::vector<Surfel>> result;

    Domain domain(myKPtr->uKCoords(myKPtr->lowerCell()), myKPtr->uKCoords(myKPtr->upperCell()));
    for (auto point : domain) {
      auto surfel = myKPtr->sCell(point, true);
      if (!KPreSpace::sIsSurfel(surfel)) continue;
      
      auto pixel = KPreSpace::interiorVoxel(surfel);
      bool hasGoodOrientation = myImgPtr->domain().isInside(myKPtr->interiorVoxel(surfel));
      if (!hasGoodOrientation) surfel = myKPtr->sOpp(surfel);

      DGtal::trace.info() << "Surfel " << surfel << ".";
      if (isVisited(surfel)) {
        DGtal::trace.info() << " Visited, skip." << std::endl;
        continue;
      };
      

      
      
      Value innerValue = myImgPtr->operator()(KPreSpace::interiorVoxel(surfel));
      DGtal::trace.info() << " inner " << innerValue;
      Value outerValue = myImgPtr->operator()(KPreSpace::exteriorVoxel(surfel));
      DGtal::trace.info() << ", outer " << outerValue;
      if (innerValue == outerValue) {
        DGtal::trace.info() << " Not a boundary, skip." << std::endl;
        continue;
      };

      auto predicate = Predicate(ksExpanded, myImgPtr, innerValue, outerValue);
      std::vector<Surfel> arc;
      DGtal::trace.info() << " Tracking... " << std::endl;
      DGtal::Surfaces<KSpace>::track2DSurface(arc, ksExpanded, sAdj, predicate, surfel);
      
      DGtal::trace.info() << "arc length " << arc.size() << ".";
      markVisited(arc);
      DGtal::trace.info() << arc;
      DGtal::trace.info() << " Marked visited";
      result.push_back(arc);
      DGtal::trace.info() << " & pushed." << std::endl;
    }
    return result;
    /*
    auto allCells = boost::make_iterator_range(domain.begin(), domain.end());
    auto newSurfels = allCells | boost::adaptors::filter([this](KSpace::Cell& cell) {
      return KPreSpace::uIsSurfel(cell) && !this->readAheadCells.contains(cell);
    }) |
    boost::adaptors::transform([this](KSpace::Cell& cell) {
      return this->myKPtr->signs(cell, true);
    });

    for (const Surfel s : newSurfels) {

    }
    */
  }

// protected:
  void markVisited(const std::vector<Surfel>& surfels) {
    for (auto s : surfels) {
      readAheadCells.insert(myKPtr->unsigns(s));
    }
  }
  
  bool isVisited(const Surfel& aSurfel) {
    return readAheadCells.contains(myKPtr->unsigns(aSurfel));
  }

private:
  const KSpace* myKPtr;
  const KSpace ksExpanded;
  const Image* myImgPtr;
  std::unordered_set<typename KSpace::Cell> readAheadCells;
};

}