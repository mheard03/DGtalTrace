#pragma once
#include <DGtal/helpers/StdDefs.h>
#include <algorithm>
#include <boost/range/adaptor/transformed.hpp>
#include <cstdio>
#include <exception>
#include <iostream>
#include <optional>
#include <ranges>
#include <type_traits>
#include <unordered_set>
#include <boost/range/adaptors.hpp>

#include <DGtal/base/Common.h>
#include <DGtal/base/Assert.h>
#include <DGtal/base/Trace.h>


#include <DGtal/io/Color.h>
#include <DGtal/images/ImageSelector.h>
#include <DGtal/topology/SurfelAdjacency.h>
#include <DGtal/topology/helpers/Surfaces.h>
#include <DGtal/topology/KhalimskyCellHashFunctions.h>
#include <DGtal/geometry/curves/GridCurve.h>
#include <utility>
#include "concepts/CXCellularGridSpaceND.cpp"
#include "concepts/dgtal/images/CConstImage.cpp"
#include "util/ConstSurfelPredicate.cpp"
#include "util/SurfelFollowers.cpp"
#include "ContourPredicate.cpp"

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
  typedef typename functors::ContourPredicate<KSpace, Image> Predicate;
  typedef typename TImage::Value ImageValue;

  ArcTracker(DGtal::ConstAlias<KSpace> aKSpace, DGtal::ConstAlias<Image> anImage) :
    myKPtr(&aKSpace), mySF(&aKSpace), myImgPtr(&anImage), myEdgePredicate(myKPtr, myImgPtr) {}

  void trackSurface(std::vector<Surfel>& aContour, const Surfel& aSurfel) 
  requires (KSpace::dimension == 2) {
    ASSERT_MSG(myKPtr->sIsSurfel(aSurfel), "aSurfel is not a surfel");
    typedef typename KSpace::SCell SCell;

    aContour.clear(); // boundary being extracted.

    // track backwards first
    trackSurface(aContour, aSurfel, false);
    std::reverse(aContour.begin(), aContour.end());
    if (aContour.size() > 0) {
      // DGtal::trace.info() << "  found " << aContour.size() << " tracking backwards" << std::endl;
    }
    
    // return if it's already a closed loop
    if (aContour.size() >= 4) {
      Surfel first = aContour.front();
      Surfel last = aContour.back();
      SCell startPoint = myKPtr->sIndirectIncident(first, *myKPtr->sDirs(first));
      SCell endpoint = myKPtr->sDirectIncident(last , *myKPtr->sDirs(last));      
      if (startPoint == endpoint) return;
    }

    // we will be tracking forward! pop aSurfel from the end of the list, because it's about to get re-added
    if (aContour.size() > 0) aContour.pop_back();
    trackSurface(aContour, aSurfel, true);
    if (aContour.size() > 0) {
      // DGtal::trace.info() << "  found " << aContour.size() << " total" << std::endl;
    }
  }

  // translation of DGtal::Surfaces<KSpace>::track2DSurface
  void trackSurface(std::vector<Surfel>& aContour, const Surfel& aSurfel, const bool fwd) {
    const Surfel startSurfel = aSurfel;
    Surfel next = startSurfel;

    // std::vector<Surfel> tmp;
    do {
      aContour.push_back(next);
      if (aSurfel != startSurfel && isVisited(next)) throw "fkn how";
      readAheadCells.insert(myKPtr->unsigns(next));
    }
    while ((fwd ? mySF.nextUnique(next, myEdgePredicate) : mySF.prevUnique(next, myEdgePredicate)) && next != startSurfel);
  }

  std::vector<std::vector<Surfel>> readImage() {
    std::vector<std::vector<Surfel>> result;

    int count = 0;
    Domain domain(myKPtr->uKCoords(myKPtr->lowerCell()), myKPtr->uKCoords(myKPtr->upperCell()));
    DGtal::trace.info() << "Domain " << domain << std::endl;
    
    std::vector<Surfel> nextArc;
    for (auto point : domain) {
      auto surfel = myKPtr->sCell(point, true);     
      if (!KPreSpace::sIsSurfel(surfel)) continue;
      if (isVisited(surfel)) continue;
      myEdgePredicate.init(surfel);
      if (!myEdgePredicate.hasContrast()) continue;
      
      
      trackSurface(nextArc, surfel);
      if (nextArc.size() > 0) {
        result.push_back(nextArc);
      }
      /*
      if (nextArc.size() > 0) {
        DGtal::trace.info() << "Found arc of size " << nextArc.size() << "at surfel" << surfel << std::endl;
        DGtal::GridCurve<KSpace> gc(myKPtr);
        auto isValid = gc.initFromSCellsVector(nextArc);
        DGtal::trace.info() << "  Valid GridCurve? " << isValid << "; " << gc << std::endl;        
        result.push_back(nextArc);
      }
      */
    }
    return result;

  }

  bool isVisited(const Surfel& aSurfel) {
    return readAheadCells.contains(myKPtr->unsigns(aSurfel));
  }

private:
  const KSpace* myKPtr;
  SurfelFollowers<KSpace> mySF;
  const Image* myImgPtr;
  Predicate myEdgePredicate;  
  std::unordered_set<typename KSpace::Cell> readAheadCells;

  /*
  KSpace getUnbounded(const KSpace& ks) {
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
  */
};

}