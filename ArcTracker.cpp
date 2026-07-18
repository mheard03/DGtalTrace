
#include <iostream>
#include <ranges>
#include <unordered_set>
#include <DGtal/base/Common.h>
#include <DGtal/io/Color.h>
#include <DGtal/images/ImageSelector.h>
#include <DGtal/topology/helpers/BoundaryPredicate.h>
#include <DGtal/topology/SurfelNeighborhood.h>
#include "util/concepts/CXCellularGridSpaceND.cpp"
#include "util/concepts/dgtal/CSurfelPredicate.cpp"
#include "util/concepts/dgtal/images/CConstImage.cpp"

namespace DGtalTrace {

template <concepts::CXCellularGridSpaceND TKSpace, concepts::CConstImage TImage>
requires (TKSpace::dimension == 2)
class ArcTracker {
  typedef TImage Image;
  typedef TKSpace KSpace;
  typedef typename KSpace::SCell Surfel;
  typedef typename DGtal::functors::BoundaryPredicate<KSpace, Image> Predicate;
  typedef typename TImage::Value ImageValue;

  Predicate makePredicate(const Surfel aSurfel);

  ArcTracker() {}

  ArcTracker(DGtal::ConstAlias<KSpace> aSpace, DGtal::ConstAlias<Image> anImage) :
    myKPtr(&aSpace), myImgPtr(&anImage) {}

// protected:
  template <typename TRange>
  requires std::ranges::range<TRange> && 
    std::assignable_from<Surfel, std::ranges::range_value_t<TRange>>
  void markVisited(const TRange& surfels) {
    for (Surfel& s : surfels) {
      readAheadCells.insert(myKPtr->unsigns(s));
    }
  }
  
  bool isVisited(const Surfel& aSurfel) {
    return readAheadCells.contains(myKPtr->unsigns(aSurfel));
  }

private:
  const KSpace* myKPtr;
  const Image* myImgPtr;
  std::unordered_set<typename KSpace::Cell> readAheadCells;
};

}