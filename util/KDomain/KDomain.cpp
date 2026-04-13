#pragma once
#include <DGtal/base/Assert.h>
#include <DGtal/base/Common.h>
#include <DGtal/helpers/StdDefs.h>
#include "DGtal/topology/CCellularGridSpaceND.h"
#include <DGtal/kernel/BasicPointPredicates.h>
#include <DGtal/kernel/domains/HyperRectDomain.h>
#include <DGtal/kernel/domains/Linearizer.h>
#include <DGtal/topology/KhalimskySpaceND.h>
#include <DGtal/base/CSTLAssociativeContainer.h>
#include <concepts>
#include <ranges>
#include <type_traits>
#include "../concepts/DGtalConcepts.cpp"
#include "../concepts/CCell.cpp"
#include <boost/iterator/transform_iterator.hpp>
#include <boost/concept/assert.hpp>
#include "DGtal/kernel/BasicPointPredicates.h"
#include <boost/concept/assert.hpp>
#include "RectangleND.cpp"
#include "../util/traceHelper.cpp"

namespace DGtalTrace {
namespace functors {

template <typename TPoint>
struct IsWithinAnyPointPredicate {
  typedef TPoint Point;
  typedef DGtal::functors::IsWithinPointPredicate<Point> SubPredicate;

  IsWithinAnyPointPredicate() {}

  void add(const SubPredicate& subPredicate) {
    mySubPredicates.push_back(subPredicate);
  }

  bool operator()(const Point & p ) const {
    for (const SubPredicate &subPred : mySubPredicates) {
      if (subPred(p)) return true;
    }
    return false;
  }

 private:
  std::vector<SubPredicate> mySubPredicates;
};
BOOST_CONCEPT_ASSERT (( DGtal::concepts::CPointPredicate<IsWithinAnyPointPredicate<DGtal::Z2i::Point>> ));


template <typename TKSpace, typename TPointPredicate>
requires std::same_as<typename TKSpace::Point, typename TPointPredicate::Point>
struct CellPredicate {
  typedef TKSpace KSpace;
  typedef TPointPredicate PointPredicate;
  typedef KSpace::Point Point;
  BOOST_CONCEPT_ASSERT (( DGtal::concepts::CCellularGridSpaceND<KSpace> ));
  BOOST_CONCEPT_ASSERT (( DGtal::concepts::CPointPredicate<TPointPredicate> ));
  
  CellPredicate() : myKPtr(nullptr) {}
  CellPredicate(DGtal::ConstAlias<KSpace> ks) : myKPtr(&ks) {}
  CellPredicate(DGtal::ConstAlias<KSpace> ks, const PointPredicate& predicate) : myKPtr(&ks), myPointPredicate(predicate) {}
  
  // CellPredicate(const CellPredicate& other) = default;
  // CellPredicate & operator=( CellPredicate const& other ) = default;

  template <concepts::CXXCell TCell>
  bool operator()(const TCell& c) const {
    return myPointPredicate(c.preCell().coordinates);
  }
  bool operator()(const Point& p) const {
    if (myKPtr->cIsValid(p)) return myPointPredicate(p);
    if (!myKPtr->cIsInside(p)) return false;
    return myPointPredicate(myKPtr->uCell(p).preCell().coordinates);
  }

  PointPredicate& predicate() {
    return myPointPredicate;
  }

 private:
  const KSpace* myKPtr;
  PointPredicate myPointPredicate;
};
BOOST_CONCEPT_ASSERT (( DGtal::concepts::CPointPredicate<CellPredicate<DGtal::Z2i::KSpace, DGtal::functors::TruePointPredicate<DGtal::Z2i::Point>>> ));

} // namespace functors

// typedef DGtal::Z2i::KSpace TKSpace;
template <typename TKSpace>
requires DGtalTrace::concepts::CXXCell<typename TKSpace::Cell>
class KDomain {
 public:
  typedef TKSpace KSpace;
  typedef typename KSpace::PreCellularGridSpace PKSpace;
  BOOST_CONCEPT_ASSERT(( DGtal::concepts::CCellularGridSpaceND<TKSpace> ));
  typedef typename KSpace::Space Space;
  typedef typename KSpace::Cell Cell;
  typedef typename KSpace::PreCell PreCell;
  typedef typename KSpace::Point Point;
  static constexpr DGtal::Dimension dimension = KSpace::dimension;

  typedef DGtal::HyperRectDomain<typename TKSpace::Space> Domain;
  typedef typename Domain::Vector Vector;
  typedef typename Domain::Integer Integer;
  typedef typename Domain::Size Size;
  typedef typename Domain::Dimension Dimension;
  typedef typename functors::CellPredicate<KSpace, functors::IsWithinAnyPointPredicate<Point>> Predicate;
  
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
  KDomain() : myKPtr(nullptr) {}

  KDomain(DGtal::ConstAlias<KSpace> ks, const Cell& lowerCell, const PreCell& upperCell)
    : myKPtr(&ks),
      myDomain(makeDomain(ks, lowerCell, upperCell)),
      myPredicate(ks)
  {
    using namespace DGtal::functors;

    if (!myKPtr->isValid()) throw std::invalid_argument("KDomain constructor received invalid KSpace");
    
    if (!myKPtr->isAnyDimensionPeriodic() || ks->cIsValid(PKSpace::uKCoords(upperCell))) {
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

  KDomain(const KDomain& other)
    : myKPtr(other.myKPtr),
      myDomain(other.myDomain),
      myPredicate(other.myPredicate)
  {
    ASSERT(myKPtr->isValid());
  }

  KDomain& operator=(const KDomain& other) {
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
    return PKSpace::uKCoords(c);
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
    return PKSpace::uCell(myDomain.upperBound());
  }
  bool isInside(const Cell& c) const {
    return myDomain.isInside(c.preCell().coordinates);
    // myPredicate(c);
  }
  //const Predicate& predicate() const {
  //  return myPredicate;
  //}

  // ----------------------- Display ----------------------------------------

  void selfDisplay(std::ostream& out) const {
    out << "[KDomain] = [" << myDomain.lowerBound() << "]x["
        << myDomain.upperBound() << "]";
  }

  // ----------------------- Private ----------------------------------------
// private:

  static constexpr Domain makeDomain(DGtal::ConstAlias<KSpace> ks, const Cell& lowerCell, const PreCell& upperCell) {
    Point lowerPoint = ks->uKCoords(lowerCell);
    Point upperPoint = PKSpace::uKCoords(upperCell);
    Vector domainExtent = upperCell.coordinates - lowerCell.preCell().coordinates + Point::diagonal(1);
    Vector kExtent = ks->uKCoords(ks->upperCell()) - PKSpace::uKCoords(ks->lowerCell()) + Point::diagonal(1);
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
std::ostream& operator<<(std::ostream& out, const KDomain<TKSpace>& object) {
  object.selfDisplay(out);
  return out;
}

} // namespace DGtalTrace


/*
template <DGtalTrace::concepts::CCell TCell>
inline bool IsWithinPointPredicate<TCell>::operator()(const TCell& c) const {

};

};
  return p.isUpper( myLowerBound )
    && p.isLower( myUpperBound );
}
  typedef TPoint Point;

  IsWithinPointPredicate( const Point & lowerBound,
        const Point & upperBound );

  bool operator()( const Point & p ) const;

  /// the upper bound.
  Point myLowerBound;
  /// the lower bound.
  Point myUpperBound;
};
}
}
template <> 
inline bool DGtal::functors::IsWithinPointPredicate<DGtalTrace::concepts::CXXCell<TPoint>>
::operator()( const Point & p ) const
{
  return p.isUpper( myLowerBound )
    && p.isLower( myUpperBound );
}
    */

BOOST_CONCEPT_ASSERT((DGtal::concepts::CDomain<DGtal::Z2i::Domain> ));
// BOOST_CONCEPT_ASSERT((DGtal::concepts::CDomain<DGtalTrace::KDomain<DGtal::Z2i::KSpace>> ));
