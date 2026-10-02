#pragma once
#include <DGtal/base/Common.h>
#include <DGtal/base/ConstAlias.h>
#include "../concepts/dgtal/images/CConstImage.cpp"
#include "../concepts/dgtal/CSurfelPredicate.cpp"
#include "../concepts/CXCellularGridSpaceND.cpp"
#include "../ContourPredicate.cpp"
#include "../util/KCoordsAccessor.cpp"
#include "ArcData.cpp"

namespace DGtalTrace {

template <concepts::CXCellularGridSpace2D TKSpace, concepts::CConstImage TImage>
requires std::same_as<typename TKSpace::Point, typename TImage::Point>
class Arc {
  typedef ArcData<TKSpace, TImage> ArcData;
  typedef ArcData::KSpace KSpace;
  typedef ArcData::Surfel Surfel;
  typedef ArcData::Point Point;
  typedef ArcData::KCA KCA;
  typedef ArcData::Values Values;
  
  Arc(DGtal::ConstAlias<ArcData> arcData, bool isReversed) : myArcData(arcData), myIsReversed(isReversed) {}

  inline const Surfel firstSurfel() {
    return myIsReversed ? KCA::sOpp(myArcData.lastSurfel) : myArcData.firstSurfel;
  };
  inline const Surfel lastSurfel() {
    return myIsReversed ? KCA::sOpp(myArcData.firstSurfel) : myArcData.lastSurfel;
  };
  inline const Values values() {
    return myIsReversed ? Values{myArcData.values.exterior, myArcData.values.interior} : myArcData.values;
  };
  inline const size_t size() {
    return myArcData.size;
  };

 private:
  const ArcData myArcData;
  const bool myIsReversed;
};

}