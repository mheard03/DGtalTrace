#pragma once
#include <DGtal/base/Common.h>
#include "../concepts/dgtal/images/CConstImage.cpp"
#include "../concepts/CXCellularGridSpaceND.cpp"
#include "../ContourPredicate.cpp"
#include "../util/KCoordsAccessor.cpp"

namespace DGtalTrace {

template <concepts::CXCellularGridSpace2D TKSpace, concepts::CConstImage TImage>
requires std::same_as<typename TKSpace::Point, typename TImage::Point>
struct ArcData {
  typedef ArcData<TKSpace, TImage> Self;

  typedef TKSpace KSpace;
  typedef typename KSpace::Surfel Surfel;
  typedef typename KSpace::Point Point;
  typedef KCoordsAccessor<KSpace> KCA;
  typedef TImage Image;
  typedef functors::ContourPredicate<KSpace, Image>::OptValues Values;

  const Surfel firstSurfel;  
  const Surfel lastSurfel;
  const Values values;
  const size_t size;

  ArcData(const Surfel& firstSurfel, const Surfel& lastSurfel, const Values& values, const size_t size)
    : firstSurfel(firstSurfel),
      lastSurfel(lastSurfel),
      values(values),
      size(size) {}

  static Self init(std::vector<Surfel>& surfels, const Values& values) {
    ASSERT(values.interiorValue.has_value() || values.exteriorValue.has_value());
    ASSERT(values.interiorValue.has_value() || values.exteriorValue.has_value());
    Surfel firstSurfel = surfels.front();
    Surfel lastSurfel = surfels.back();
    Self arc(firstSurfel, lastSurfel, values, surfels.size());

    // Canonical ordering: firstSurfel is the surfel with the lowermost coordinates whose interior spel has a value
    if (!values.exteriorValue.has_value()) return arc;
    if (firstSurfel == lastSurfel) return arc;
    if (values.interiorValue.has_value() && DGtal::isLower(KCA::uKCoords(firstSurfel), KCA::uKCoords(lastSurfel))) return arc;

    Values fValues{values.exteriorValue, values.interiorValue};
    return Self(KCA::sOpp(lastSurfel), KCA::sOpp(firstSurfel), fValues, arc.size);
  }

public:
  ArcData(const std::vector<Surfel>& surfels, const Values& values) : ArcData(init(surfels, values)) {}
  ArcData(const ArcData& other) = default;
  ArcData(ArcData&& other) = default;

  // const Surfel firstPointel() const; 
  // const Surfel lastPointel() const;  
};

}