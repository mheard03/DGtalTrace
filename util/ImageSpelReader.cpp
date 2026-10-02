#pragma once
#include <optional>

#include <DGtal/helpers/StdDefs.h>
#include <DGtal/base/Assert.h>
#include <DGtal/base/Common.h>
#include <DGtal/base/ConstAlias.h>

#include "../concepts/CXCellularGridSpaceND.cpp"
#include "../concepts/dgtal/images/CConstImage.cpp"
#include "KCoordsAccessor.cpp"
#include "OptValue.cpp"
#include "CellTypeTraits.cpp"
#include "KCoordsAdjuster.cpp"

namespace DGtalTrace {

template <concepts::CXCellularGridSpaceND TKSpace, concepts::CConstImage TImage>
requires std::same_as<typename TKSpace::Point, typename TImage::Point>
class ImageSpelReader {
  typedef ImageSpelReader<TKSpace, TImage> Self;

public:
  typedef TKSpace KSpace;
  typedef KCoordsAdjuster<KSpace> KCA;
  typedef typename KSpace::Cell Cell;
  typedef typename KSpace::SCell SCell;
  typedef TImage Image;
  typedef typename Image::Point Point;
  typedef typename Image::Value Value;
  typedef detail::OptValue<Value> OptValue;
  typedef std::optional<Point> OptPoint;

  struct OptValues {
    typedef OptValue OptValue;
    OptValue interior;
    OptValue exterior;

    bool operator==(const OptValues& other) const = default;
  };

private:
  typedef typename KSpace::PreCellularGridSpace KPreSpace;
  typedef typename KPreSpace::SCell SPreCell;

public:
  ImageSpelReader(DGtal::ConstAlias<KSpace> aKSpace, DGtal::ConstAlias<Image> anImage) :
    myKCA(&aKSpace), myImgPtr(&anImage) {}

  OptValue atCoord(const Point& aPoint) const {
    return (myImgPtr->domain().isInside(aPoint)) ? OptValue((*myImgPtr)(aPoint)) : std::nullopt;
  }
  inline OptValue atCoord(const OptPoint& aPoint) const {
    return (aPoint.has_value()) ? atCoord(aPoint.value()) : std::nullopt;
  }

  template <concepts::CXXCell TCell>
  OptValue atSpel(TCell aSpel) const {
    ASSERT(myKCA.dim(aSpel) == KSpace::dimension);
    if (!myKCA.isInside(aSpel)) return std::nullopt;
    return atCoord(myKCA.coords(myKCA.uCell(aSpel)));
  }

  OptValues atSurfel(const SPreCell& aSurfel) const {
    ASSERT(myKCA.dim(aSurfel) == KSpace::dimension - 1);
    auto d = KPreSpace::sOrthDir(aSurfel);
    auto interiorSpel = KPreSpace::sIncident(aSurfel, d, KPreSpace::sDirect(aSurfel, d));
    auto exteriorSpel = KPreSpace::sIncident(aSurfel, d, !KPreSpace::sDirect(aSurfel, d));
    OptPoint interiorCoords = myKCA.isValid(interiorSpel) ? KCA::coords(interiorSpel) : OptPoint();
    OptPoint exteriorCoords = myKCA.isValid(exteriorSpel) ? KCA::coords(exteriorSpel) : OptPoint();
    return OptValues{atCoord(interiorCoords), atCoord(exteriorCoords)};
  }

private:
  KCA myKCA;
  const Image* myImgPtr;
  bool myCheckImageDomain;
};


} // end namespace DGtalTrace