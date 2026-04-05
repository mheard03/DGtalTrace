#pragma once
#include <DGtal/base/Assert.h>
#include <DGtal/base/Common.h>
#include <DGtal/helpers/StdDefs.h>
#include "DGtal/topology/CCellularGridSpaceND.h"
#include <DGtal/kernel/domains/HyperRectDomain.h>
#include <DGtal/topology/KhalimskySpaceND.h>
#include <DGtal/base/CSTLAssociativeContainer.h>
#include <concepts>
#include <ranges>
#include <type_traits>
#include "concepts/DGtalConcepts.cpp"
#include <boost/iterator/transform_iterator.hpp>

namespace DGtalTrace {

template <typename TKSpace>
class KDomain {
  typedef DGtal::HyperRectDomain<typename TKSpace::Space> Domain;

  // ---- Declare PointToCell first so ConstIterator can reference it --------
private:
  struct PointToCell {
    const TKSpace* myKPtr;
    typename TKSpace::Cell operator()(typename TKSpace::Space::Point const& p) const {
      return myKPtr->uCell(p);
    }
  };

  using InnerIterator = typename Domain::ConstIterator;

public:
  using ConstIterator = boost::transform_iterator<PointToCell, InnerIterator>;

  // ---- Public interface, including ConstIterator which depends on PointToCell
public:
  typedef TKSpace KSpace;
  typedef typename KSpace::PreCellularGridSpace PKSpace;
  BOOST_CONCEPT_ASSERT(( DGtal::concepts::CCellularGridSpaceND<TKSpace> ));
  typedef typename KSpace::Space Space;
  typedef typename KSpace::Cell Cell;
  typedef typename KSpace::PreCell PreCell;
  static constexpr DGtal::Dimension dimension = KSpace::dimension;

  typedef typename Domain::Point Point;
  typedef typename Domain::Vector Vector;
  typedef typename Domain::Integer Integer;
  typedef typename Domain::Size Size;
  typedef typename Domain::Dimension Dimension;
  typedef typename Domain::Predicate Predicate;

  // ----------------------- Standard services ------------------------------

  KDomain() : myKPtr(nullptr) {}

  KDomain(DGtal::ConstAlias<KSpace> ks, const PreCell& lowerCell, const PreCell& upperCell)
    : myKPtr(&ks)
    , myDomain(
        PKSpace::uKCoords(normalizeCell(ks, lowerCell)),
        PKSpace::uKCoords(normalizeUpperCell(ks, lowerCell, upperCell))
      )
  {}

  KDomain(const KDomain& other)
    : myKPtr(other.myKPtr)
    , myDomain(other.myDomain)
  {}

  KDomain& operator=(const KDomain& other) {
    if (this != &other) {
      myKPtr = other.myKPtr;
      myDomain = other.myDomain;
    }
    return *this;
  }

  // ----------------------- Range interface --------------------------------

  ConstIterator begin() const {
    return boost::make_transform_iterator(myDomain.begin(), PointToCell{myKPtr});
  }

  ConstIterator end() const {
    return boost::make_transform_iterator(myDomain.end(), PointToCell{myKPtr});
  }

  Size size() const { return myDomain.size(); }
  bool isEmpty() const { return myDomain.isEmpty(); }

  // ----------------------- Display ----------------------------------------

  void selfDisplay(std::ostream& out) const {
    out << "[KDomain] = [" << myDomain.lowerBound() << "]x["
        << myDomain.upperBound() << "]";
  }

  // ----------------------- Private ----------------------------------------
// private:
  typedef typename Point::Component Value;

  static constexpr Cell normalizeCell(DGtal::ConstAlias<KSpace> ks, const PreCell& cell) {
    ASSERT_MSG(ks->isValid(), "ks must be a valid, initialized KSpace.");
    return ks->uCell(cell);
  }

  static constexpr PreCell normalizeUpperCell(DGtal::ConstAlias<KSpace> ks, const PreCell& lowerCell, const PreCell& upperCell) {
    ASSERT_MSG(ks->isValid(), "ks must be a valid, initialized KSpace.");
    if (!ks->isAnyDimensionPeriodic()) return normalizeCell(ks, upperCell);

    const Point lowerBound = ks->uKCoords(ks->lowerCell());
    const Point upperBound = ks->uKCoords(ks->upperCell());
    const Point normLowerCoords = ks->uKCoords(normalizeCell(ks, lowerCell));
    const Vector inputDistance = PKSpace::uKCoords(upperCell) - PKSpace::uKCoords(lowerCell);
    PreCell result = upperCell;

    for (DGtal::Dimension k = 0; k < KSpace::dimension; k++) {
      if (!ks->isSpacePeriodic(k)) continue;
      Value dimWidth = upperBound[k] - lowerBound[k];
      Value distance = inputDistance[k];
      if (distance < 0) distance += dimWidth;
      distance = std::min(inputDistance[k], dimWidth);
      PKSpace::uSetKCoord(result, k, normLowerCoords[k] + distance);
    }
    return result;
  }
  
 private:
  const KSpace* myKPtr;
  Domain myDomain;
};

template <typename TKSpace>
std::ostream& operator<<(std::ostream& out, const KDomain<TKSpace>& object) {
  object.selfDisplay(out);
  return out;
}

} // namespace DGtalTrace