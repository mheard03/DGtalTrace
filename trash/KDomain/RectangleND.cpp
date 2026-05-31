#pragma once
#include <boost/concept/assert.hpp>
#include <DGtal/helpers/StdDefs.h>
#include <DGtal/base/Common.h>
#include "../concepts/DGtalTraceConcepts.cpp"
#include "../traceHelper.cpp"

namespace DGtalTrace {
template <typename TPoint> requires DGtal::IsAPointVector<TPoint>::value
struct RectangleND {
  typedef TPoint Point;
  typedef Point Vector;
  static const DGtal::Dimension dimension = Point::dimension;
  typedef typename TPoint::Component Value;
  typedef RectangleND<Point> Self;

  RectangleND() {}
  RectangleND(const Point& lower, const Point& upper) : myLower(lower), myUpper(upper) {}
  RectangleND(const std::pair<Point, Point>& corners) : myLower(corners.first), myUpper(corners.second) {}

  operator std::pair<Point, Point>() const {
    return { myLower, myUpper };
  }

  std::optional<Self> intersection(const Self& other) const {
    Self result;
    for (DGtal::Dimension k = 0; k < Point::dimension; ++k) {
      result.myLower[k] = std::max(myLower[k], other.myLower[k]);
      result.myUpper[k] = std::min(myUpper[k], other.myUpper[k]);
      if (result.isEmpty()) return std::nullopt; // disjoint
    }
    return result;
  }

  bool contains(const Self& other) const {
    return isLower(myLower, other.myLower) && isUpper(myUpper, other.myUpper);
  }
  bool contains(const Point& point) const {
    return (myLower <= point) && (myUpper >= point);
  }

  bool isEmpty() const {
    return !myLower.isLower(myUpper);
  }

  const Point& lower() const { return myLower; }
  const Point& upper() const { return myUpper; }

  const Vector extent() const { 
    return myUpper - myLower + Point::diagonal(1);
  }

  void selfDisplay(std::ostream& out) const {
    out << "[Rect] = " << myLower << "|" << myUpper;
  }

 protected:
  
  

 private:
  Point myLower;
  Point myUpper;
};

template <typename TPoint>
std::ostream& operator<<(std::ostream& out, const RectangleND<TPoint>& object) {
  object.selfDisplay(out);
  return out;
}

} // namespace DGtalTrace
