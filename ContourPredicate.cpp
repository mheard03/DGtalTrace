#pragma once
#include <iostream>
#include <optional>

#include <DGtal/helpers/StdDefs.h>
#include <DGtal/base/Assert.h>

#include "concepts/CXCellularGridSpaceND.cpp"
#include "concepts/dgtal/images/CConstImage.cpp"
#include "util/ImageSpelReader.cpp"
// #include "util/readImage.cpp"

namespace DGtalTrace {
namespace functors {

template <concepts::CXCellularGridSpaceND TKSpace, concepts::CConstImage TImage>
  requires(std::same_as<typename TImage::Point, typename TKSpace::Point>)
class ContourPredicate {
  typedef ContourPredicate<TKSpace, TImage> Self;

public:
  typedef TKSpace KSpace;
  typedef TImage Image;

private:
  typedef ImageSpelReader<KSpace, Image> Reader;

public:
  typedef typename KSpace::Surfel Surfel;
  typedef typename KSpace::Point Point;
  typedef Reader::OptValues OptValues;

public:
  ContourPredicate(DGtal::ConstAlias<KSpace> aSpace, DGtal::ConstAlias<Image> anImage)
      : myReader(&aSpace, &anImage) {}

  ContourPredicate(const Self &other)
      : myReader(other.myReader),
        myValues(other.myValues) {}

  Self operator=(const Self &other) {
    if (this != &other) {
      myReader = other.myReader;
      myValues = other.myValues;
    }
    return *this;
  }

  void init(const Surfel &aSurfel) {
    myValues = myReader.atSurfel(aSurfel);
  }
  void init(const OptValues &values) {
    myValues = values;
  }
  void init(const OptValues::OptValue &interiorValue, const OptValues::OptValue &exteriorValue) {
    init(OptValues{interiorValue, exteriorValue});
  }

  bool operator()(const Surfel &aSurfel) const {
    OptValues values = myReader.atSurfel(aSurfel);
    return myValues == values;
  }

  void selfDisplay(std::ostream &out) const {
    out << "[ContourPredicate " << myValues.interior << ", " << myValues.exterior << " ]";
  }

  bool hasContrast() const {
    return myValues.interior != myValues.exterior;
  }

  const OptValues& values() const {
    return myValues;
  }

private:
  ImageSpelReader<KSpace, Image> myReader;
  OptValues myValues;
};

} // namespace functors
} // namespace DGtalTrace

template <typename TKSpace, typename TImage>
std::ostream &
operator<<(std::ostream &out,
           const DGtalTrace::functors::ContourPredicate<TKSpace, TImage>
               &object) {
  object.selfDisplay(out);
  return out;
}
