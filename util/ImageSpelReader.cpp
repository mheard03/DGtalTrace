#pragma once
#include <optional>

#include <DGtal/helpers/StdDefs.h>
#include <DGtal/base/Assert.h>
#include <DGtal/base/Common.h>
#include <DGtal/base/ConstAlias.h>

#include "../concepts/CXCellularGridSpaceND.cpp"
#include "../concepts/dgtal/images/CConstImage.cpp"
#include "CellTypeTraits.cpp"
#include "KCoordsAdjuster.cpp"

namespace DGtalTrace {

namespace {
template <typename T> struct is_optional : std::false_type {};
template <typename T> struct is_optional<std::optional<T>> : std::true_type {};
template <typename T> concept IsOptional = is_optional<std::remove_cvref_t<T>>::value;

template <typename T>
using OptValue_T = std::conditional_t<IsOptional<T>, T, std::optional<T>>
;
} // end anonymous namespace

template <concepts::CXCellularGridSpaceND TKSpace, concepts::CConstImage TImage>
requires std::same_as<typename TKSpace::Point, typename TImage::Point>
class ImageSpelReader {
public:
  typedef TKSpace KSpace;
  typedef typename KSpace::Cell Cell;
  typedef typename KSpace::SCell SCell;
  typedef TImage Image;
  typedef typename Image::Point Point;
  typedef typename Image::Value Value;
  typedef OptValue_T<Value> OptValue;
  typedef std::pair<OptValue, OptValue> OptValuePair;
  typedef std::optional<Point> OptPoint;
  typedef std::pair<OptPoint, OptPoint> OptPointPair;

private:
  typedef typename KSpace::PreCellularGridSpace KPreSpace;
  typedef typename KPreSpace::SCell SPreCell;

public:
  ImageSpelReader(DGtal::ConstAlias<KSpace> aKSpace, DGtal::ConstAlias<Image> anImage) :
    myKPtr(&aKSpace), myImgPtr(&anImage) {}

  OptValue atPoint(const Point& aPoint) const {
    return (myImgPtr->domain().isInside(aPoint)) ? OptValue((*myImgPtr)(aPoint)) : std::nullopt;
  }

protected:
  inline OptValue atPoint(const OptPoint& aPoint) const {
    return (aPoint.has_value()) ? OptValue((*myImgPtr)(aPoint.value())) : std::nullopt;
  }

public:
  OptValuePair atSurfel(const SPreCell& aSurfel) const {
    OptPointPair points = pointsAtSurfel(aSurfel);
    return { atPoint(points.first), atPoint(points.second) };
  }

  OptPointPair pointsAtSurfel(const SPreCell& aSurfel) const {
    ASSERT_MSG(KPreSpace::sIsSurfel(aSurfel), "aSurfel is not a surfel");
    DGtal::Dimension orthDim = KPreSpace::sOrthDir(aSurfel);
    bool interiorDirection = KPreSpace::sDirect(aSurfel, orthDim);

    return {
      preSpelToPoint(KPreSpace::sIncident(aSurfel, orthDim, interiorDirection)),
      preSpelToPoint(KPreSpace::sIncident(aSurfel, orthDim, !interiorDirection))
    };
  }

protected:
  OptPoint preSpelToPoint(const SPreCell& cell) const {
    return (myKPtr->sIsInside(cell)) ? OptPoint(myKPtr->sCoords(myKPtr->sCell(cell))) : std::nullopt;
  }

private:
  const KSpace* myKPtr;
  const Image* myImgPtr;
  bool myCheckImageDomain;
};


} // end namespace DGtalTrace