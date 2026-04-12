#pragma once
#include <DGtal/base/Assert.h>
#include <DGtal/base/Common.h>
#include <DGtal/helpers/StdDefs.h>
#include "DGtal/topology/CCellularGridSpaceND.h"
#include <DGtal/kernel/domains/HyperRectDomain.h>
#include <DGtal/kernel/domains/HyperRectDomain_Iterator.h>
#include <DGtal/topology/KhalimskySpaceND.h>
#include <DGtal/base/CSTLAssociativeContainer.h>
#include <concepts>
#include <ranges>
#include <type_traits>
#include "concepts/DGtalConcepts.cpp"
#include <boost/iterator/transform_iterator.hpp>

namespace DGtalTrace {

namespace detail {

/*
typedef DGtal::KhalimskySpaceND<2>::Point TPoint;
typedef DGtal::KhalimskySpaceND<2>::Point TPoint;
// template <typename TPoint>
class HyperRectDomain_Range {
 private:
  typedef TPoint Point;
  typedef DGtal::HyperRectDomain_Iterator<TPoint> DomainIterator;
  // typedef DomainIterator::

  DomainIterator b;
  auto start = b.begin();
      IteratorRange() = default;
    IteratorRange(Iter first, Iter last)
        : first_(first), last_(last) {}

    auto begin() const { return first_; }
    auto end()   const { return last_; }

private:
    Iter first_{};
    Iter last_{};
};
}
  
 public:
  using Iterator = DGtal::HyperRectDomain_Iterator<Point>;

    static constexpr std::ptrdiff_t total = 
        static_cast<std::ptrdiff_t>(InnerCap * OuterCap);

    explicit FlatStaticRange(const Outer& outer) : myOuter(&outer) {}

    iterator begin() const { return { myOuter, 0 };     }
    iterator end()   const { return { myOuter, total };  }

    static constexpr std::ptrdiff_t size()  { return total; }
    static constexpr bool           empty() { return total == 0; }

private:
    const Outer* myOuter;
};
*/

typedef DGtal::KhalimskySpaceND<2> TKSpace;
// template <typename TKSpace>
class KDomainBase {
  typedef DGtal::HyperRectDomain<typename TKSpace::Space> Domain;
 public:
  typedef TKSpace KSpace;
  typedef KSpace::PreCellularGridSpace PKSpace;
  BOOST_CONCEPT_ASSERT(( DGtal::concepts::CCellularGridSpaceND< TKSpace > ));
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

 private:
  typedef typename std::ranges::transform_view<typename Domain::ConstIterator, Point> TransformView;
 public:
  typedef typename TransformView::Iterator

 public:

 // private:
  static constexpr Cell normalizeCell(DGtal::ConstAlias<KSpace> ks, const PreCell &cell) {
    ASSERT_MSG(ks->isValid(), "ks must be a valid, initialized KSpace.");
    return ks->uCell(cell);
  }

  static constexpr PreCell normalizeUpperCell(DGtal::ConstAlias<KSpace> ks, const PreCell &lowerCell, const PreCell &upperCell) {
    ASSERT_MSG(ks->isValid(), "ks must be a valid, initialized KSpace.");
    if (!ks->isAnyDimensionPeriodic()) return normalizeCell(ks, upperCell);

    typedef typename Point::Component Value;
    const Point lowerBound = ks->uKCoords(ks->lowerCell());
    const Point upperBound = ks->uKCoords(ks->upperCell());
    const Point normLowerCoords = ks->uKCoords(normalizeCell(ks, lowerCell));
    const Vector inputDistance = PKSpace::uKCoords(upperCell) - PKSpace::uKCoords(lowerCell);
    PreCell result = upperCell;

    for (DGtal::Dimension k = 0; k < KSpace::dimension; k++) {
      if (!ks->isSpacePeriodic(k)) continue; // nothing to do
      Value dimWidth = upperBound[k] - lowerBound[k];
      Value distance = inputDistance[k];
      if (distance < 0) distance += dimWidth;
      distance = std::min(inputDistance[k], dimWidth);
      PKSpace::uSetKCoord(result, k, normLowerCoords[k] + distance);
    }
    return result;
  }
  
 public:
  KDomainBase() : myKPtr(nullptr) {}

  KDomainBase(DGtal::ConstAlias<KSpace> ks, const PreCell &lowerCell, const PreCell &upperCell) :
    myKPtr(&ks), 
    myDomain(
      PKSpace::uKCoords(normalizeCell(ks, lowerCell)),
      PKSpace::uKCoords(normalizeUpperCell(ks, lowerCell, upperCell))
    ) {};

  KDomainBase(const KDomainBase& other) :
    myKPtr(other.myKPtr),
    myDomain(other.myDomain) {}

  KDomainBase& operator=(const KDomainBase& other) {
    if (this != &other) {
      myKPtr = other.myKPtr;
      myDomain = other.myDomain;
    }
    return *this;
  }
 
  void selfDisplay(std::ostream & out) const {
    out << "[KDomain] = [" << myDomain.lowerBound() << "]x["
        << myDomain.upperBound() << "]";
  }

 protected:
  const KSpace *myKPtr;
  const Domain myDomain;
  
};

} // namespace detail

// typedef DGtal::KhalimskySpaceND<2> TKSpace;
template <typename TKSpace>
class KDomain : public detail::KDomainBase<TKSpace> {
  using detail::KDomainBase<TKSpace>::KDomainBase;
};

} // namespace DGtalTrace

namespace DGtal {
  template <typename TKSpace>
  std::ostream& operator<< ( std::ostream& out, const DGtalTrace::KDomain<TKSpace> & object ) {
  object.selfDisplay(out);
  return out;
}

}
