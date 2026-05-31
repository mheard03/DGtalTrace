#pragma once
#include <concepts>
#include <boost/concept/assert.hpp>
#include <boost/iterator/transform_iterator.hpp>
#include <DGtal/base/Common.h>
#include <DGtal/helpers/StdDefs.h>
#include <DGtal/kernel/CPointPredicate.h>
#include <DGtal/base/CSTLAssociativeContainer.h>
#include <DGtal/topology/CCellularGridSpaceND.h>
#include "../concepts/CCell.cpp"
#include <DGtal/kernel/domains/HyperRectDomain.h>
#include <DGtal/kernel/domains/Linearizer.h>
#include <DGtal/topology/KhalimskySpaceND.h>
#include <optional>
#include <ranges>
#include "RectangleND.cpp"
#include "PointPredicateWrapperCellPredicate.cpp"
#include "IsWithinAnyPointPredicate.cpp"
#include "../traceHelper.cpp"

namespace DGtalTrace {

// typedef DGtal::Z2i::KSpace TKSpace;
template <typename TKSpace>
requires DGtalTrace::concepts::CXXCell<typename TKSpace::Cell>
class KRectangle {
 public:
  typedef TKSpace KSpace;
  typedef typename KSpace::PreCellularGridSpace KPreSpace;
  BOOST_CONCEPT_ASSERT(( DGtal::concepts::CCellularGridSpaceND<TKSpace> ));
  typedef typename KSpace::Space Space;
  typedef typename KSpace::Cell Cell;
  typedef typename KSpace::PreCell PreCell;
  typedef typename KSpace::Point Point;
  typedef typename Point::Component Component;
  static constexpr DGtal::Dimension dimension = KSpace::dimension;

  typedef DGtal::HyperRectDomain<typename TKSpace::Space> Domain;
  typedef typename Domain::Vector Vector;
  typedef typename Domain::Integer Integer;
  typedef typename Domain::Size Size;
  typedef typename Domain::Dimension Dimension;
  typedef typename DGtal::functors::PointPredicateWrapperCellPredicate<KSpace, DGtal::functors::IsWithinAnyPointPredicate<Point>> Predicate;
  typedef DGtal::NumberTraits<typename Point::Component> NT;
  
 private:
  typedef RectangleND<Point> Rect;
  typedef DGtal::Linearizer<Domain> Linearizer;
 
  // ---- Declare PointToCell first so ConstIterator can reference it --------
  struct PointToCell {
    const TKSpace* myKPtr;
    typename TKSpace::Cell operator()(typename TKSpace::Space::Point const& p) const {
      return myKPtr->uCell(p);
    }
  };

  using InnerIterator = typename Domain::ConstIterator;

 public:
  using ConstIterator = boost::transform_iterator<PointToCell, InnerIterator>;
  

  // ----------------------- Standard services ------------------------------
  KRectangle() : myKPtr(nullptr) {}

  KRectangle(DGtal::ConstAlias<KSpace> ks, const Cell& lowerCell, const PreCell& upperCell)
    : myKPtr(&ks),
      myDomain(makeDomain(ks, lowerCell, upperCell)),
      myPredicate(ks)
  {
    using namespace DGtal::functors;

    if (!myKPtr->isValid()) throw std::invalid_argument("KDomain constructor received invalid KSpace");
    
    if (!myKPtr->isAnyDimensionPeriodic() || ks->cIsValid(KPreSpace::uKCoords(upperCell))) {
      myPredicate.predicate().add(myDomain.predicate());
    }
    else {
      auto rects = splitDomain();
      DGtal::trace.info() << "adding predicates for " << rects.size() << " rects: ";
      for (auto rect : rects) {
        auto sub = IsWithinPointPredicate(rect.lower(), rect.upper());
        myPredicate.predicate().add(sub);
        DGtal::trace.info() << rect << " ";
      }
      DGtal::trace.info() << std::endl;    
    }
  }

  KRectangle(const KRectangle& other)
    : myKPtr(other.myKPtr),
      myDomain(other.myDomain),
      myPredicate(other.myPredicate)
  {
    ASSERT(myKPtr->isValid());
  }

  KRectangle& operator=(const KRectangle& other) {
    if (this != &other) {
      myKPtr = other.myKPtr;
      myDomain = other.myDomain;
      myPredicate = other.myPredicate;
    }
    return *this;
  }

  Cell operator[](Size n) const {
    return PointToCell{myKPtr}(myDomain.begin()[n]);
  }
  Cell operator[](const Domain::Point& p) const {
    return myKPtr->uCell(p);
  }

  Domain::Point getPoint(const Cell& c) const {
    return KPreSpace::uKCoords(c);
  }
  Domain::Point getPoint(Size n) const {
    return myDomain.begin()[n];
  }

  Size getIndex(const Domain::Point& p) const {
    return Linearizer::getIndex(p, myDomain);
  }
  Size getIndex(const Cell& c) const {
    return getIndex(myKPtr->uKCoords(c));
  }

  const Domain& domain() const {
    return myDomain;
  }

  // ----------------------- Range interface --------------------------------

  ConstIterator begin() const {
    return boost::make_transform_iterator(myDomain.begin(), PointToCell{myKPtr});
  }
  ConstIterator begin(const Cell& c) const {
    return boost::make_transform_iterator(myDomain.begin(getPoint(c)), PointToCell{myKPtr});
  }

  ConstIterator end() const {
    return boost::make_transform_iterator(myDomain.end(), PointToCell{myKPtr});
  }

  Size size() const { return myDomain.size(); }
  bool isEmpty() const { return myDomain.isEmpty(); }

  // ----------------------- Domain concept ---------------------------------

  const Cell lowerBound() const {
    return myKPtr->uCell(myDomain.lowerBound());
  }
  const PreCell upperBound() const {
    return KPreSpace::uCell(myDomain.upperBound());
  }
  bool isInside(const Cell& c) const {
    // return myDomain.isInside(c.preCell().coordinates);
    return myPredicate(c);
  }
  const Predicate& predicate() const {
    return myPredicate;
  }

  // ----------------------- Bounds helpers ---------------------------------

  std::optional<Cell> uFirst(const PreCell& p, DGtal::Dimension k) const {
    Point pResult = myDomain.lowerBound();
    for (DGtal::Dimension k = 0; k < dimension; k++) {
      if (NT::odd(pResult[k]) != NT::odd(KPreSpace::uKCoord(p, k))) pResult[k] += 1;
    }
    Cell result;
    KPreSpace::uSetKCoords(result, pResult);
    return isInside(result) ? result : std::nullopt;
  }
  std::optional<Cell> uLast(const PreCell& p, DGtal::Dimension k) const {
    Point pResult = myDomain.upperBound();
    for (DGtal::Dimension k = 0; k < dimension; k++) {
      if (NT::odd(pResult[k]) != NT::odd(KPreSpace::uKCoord(p, k))) pResult[k] -= 1;
    }
    Cell result;
    KPreSpace::uSetKCoords(result, pResult);
    return isInside(result) ? result : std::nullopt;
  }

  // ----------------------- Display ----------------------------------------

  void selfDisplay(std::ostream& out) const {
    out << "[KDomain] = [" << myDomain.lowerBound() << "]x["
        << myDomain.upperBound() << "]";
  }

  // ----------------------- Construction helpers ---------------------------
 private:

  static constexpr Domain makeDomain(DGtal::ConstAlias<KSpace> ks, const Cell& lowerCell, const PreCell& upperCell) {
    Point lowerPoint = ks->uKCoords(lowerCell);
    Point upperPoint = KPreSpace::uKCoords(upperCell);
    Vector domainExtent = upperCell.coordinates - lowerCell.preCell().coordinates + Point::diagonal(1);
    Vector kExtent = ks->uKCoords(ks->upperCell()) - KPreSpace::uKCoords(ks->lowerCell()) + Point::diagonal(1);
    DGtal::trace.info() << "kExtent " << kExtent << "; domainExtent: " << domainExtent << std::endl; 

    if (!lowerPoint.isLower(upperPoint)) throw std::invalid_argument("KDomain's upperCell must be above its lowerCell");
    if (!ks->cIsValid(lowerPoint)) throw std::invalid_argument("KDomain's lowerCell was outside the bounds of the KSpace's lower and upper cells");
    if (!ks->cIsInside(upperPoint)) throw std::invalid_argument("KDomain's upperCell was outside the KSpace");

    for (DGtal::Dimension k = 0; k < KSpace::dimension; k++) {
      if (kExtent[k] < domainExtent[k]) throw std::invalid_argument("KDomain extent exceeds KSpace size in dimension " + std::to_string(k));
      if (kExtent[k] == domainExtent[k]) {
        if (lowerPoint[k] != ks->uKCoords(ks->lowerCell())[k] || upperPoint[k] != ks->uKCoords(ks->upperCell())[k]) {
          throw std::invalid_argument("KDomain with extent equal to KSpace size must have lower and upper bounds matching the KSpace's lower and upper bounds in dimension " + std::to_string(k));
        }
      }
    }
    return Domain(lowerPoint, upperPoint);
  }

  std::vector<Rect> splitDomain() const {
    using namespace DGtal;
    Rect domainRect(myDomain.lowerBound(), myDomain.upperBound());
    Rect kRect(myKPtr->uKCoords(myKPtr->lowerCell()), myKPtr->uKCoords(myKPtr->upperCell()));
    trace.info() << "splitDomain (" << kRect << ", " << domainRect << ")" << std::endl;

    if (!myKPtr->isAnyDimensionPeriodic()) return { domainRect };
    if (kRect.contains(domainRect)) return { domainRect };

    trace.info() << "continuing splitDomain (" << kRect << ", " << domainRect << ")" << std::endl;
    std::array<std::vector<int>, KSpace::dimension> periodicity = {};
    periodicity.fill({0});
    for (DGtal::Dimension k = 0; k < KSpace::dimension; k++) {
      if (myKPtr->isSpacePeriodic(k)) periodicity[k].push_back(1);
    }

    std::vector<Rect> rects;
    Vector kExtent = kRect.extent();
    trace.info() << "kExtent " << kExtent << std::endl;
    for (auto const& tuple : std::apply(std::views::cartesian_product, periodicity)) {
      Vector v = std::make_from_tuple<Vector>(tuple);
      Rect offsetRect = { 
        domainRect.lower() - v * kExtent, 
        domainRect.upper() - v * kExtent
      };
      trace.info() << "v" << v << ", offset " << (v * kExtent) << ", offsetRect" << offsetRect << std::endl;
      auto intersection = kRect.intersection(offsetRect);
      if (intersection && !intersection->isEmpty()) {
        rects.push_back(*intersection);
      }
    }


    for (auto rect : rects) {
      trace.info() << rect.lower() << "->" << rect.upper() << "; ";
    }
    trace.info() << std::endl;
    return rects;
  }
  
 private:
  const KSpace* myKPtr;
  Domain myDomain;
  Predicate myPredicate;
};

template <typename TKSpace>
std::ostream& operator<<(std::ostream& out, const KRectangle<TKSpace>& object) {
  object.selfDisplay(out);
  return out;
}

} // namespace DGtalTrace
