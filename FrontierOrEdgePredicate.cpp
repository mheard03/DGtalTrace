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
  typedef typename KSpace::Surfel Surfel;
  typedef typename KSpace::Point Point;

private:
  typedef ImageSpelReader<KSpace, Image> Reader;
  typedef Reader::OptValuePair OptValuePair;

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

  bool operator()(const Surfel &aSurfel) const {
    auto values = myReader.atSurfel(aSurfel);
    return values == myValues;
  }

  void selfDisplay(std::ostream &out) const {
    out << "[FrontierOrEdgePredicate " << myValues.first << ", " << myValues.second << " ]";
  }

  bool hasContrast() const {
    return myValues.first != myValues.second;
  }

private:
  ImageSpelReader<KSpace, Image> myReader;
  OptValuePair myValues;
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
