#pragma once
#include <DGtal/base/Assert.h>
#include <DGtal/base/Common.h>
#include <DGtal/base/FunctorHolder.h>
#include <DGtal/helpers/StdDefs.h>
#include <DGtal/topology/CCellularGridSpaceND.h>
#include <DGtal/topology/KhalimskySpaceND.h>
#include <DGtal/kernel/BasicPointPredicates.h>
#include <DGtal/kernel/PointFunctorHolder.h>
#include <ranges>
#include <concepts>
#include "../concepts/DGtalConcepts.cpp"
#include "../concepts/CCell.cpp"
#include <boost/concept/assert.hpp>
#include <type_traits>

namespace DGtalTrace {
namespace functors {

template <typename TSource, typename TTarget>
class PairedConversionFunctor;

template <typename TSource, typename TTarget>
class ConversionFunctor {
 public:
  typedef TSource Source;
  typedef TTarget Target;
  typedef TTarget Value;
  virtual Target operator()(const Source& source) const {
    return (Target)source;
  }

  friend class PairedConversionFunctor<Source, Target>;
  friend class PairedConversionFunctor<Target, Source>;
  friend class ConversionFunctor<Target, Source>;
};

namespace concepts {
  template <typename T>
  concept CHasMakeReverser = requires (const T t) {
    typename T::Source;
    typename T::Target;
    { t.makeReverser() } -> std::same_as<ConversionFunctor<typename T::Target, typename T::Source>>;
  };
}

template <typename TSource, typename TTarget>
//requires (std::copy_constructible<ConversionFunctor<TSource, TTarget>> &&
//          std::copy_constructible<ConversionFunctor<TTarget, TSource>>)
class PairedConversionFunctors {
 public:
  typedef TSource Source;
  typedef TTarget Target;
  typedef ConversionFunctor<TSource, TTarget> SourceToTargetFunctor;
  typedef ConversionFunctor<TTarget, TSource> TargetToSourceFunctor;

  PairedConversionFunctors(const SourceToTargetFunctor& forward, 
                           const TargetToSourceFunctor& reverse)
    : myForward(forward), 
      reverse(reverse) {}
  
  PairedConversionFunctors(const SourceToTargetFunctor& forward) 
      requires concepts::CHasMakeReverser<SourceToTargetFunctor>
    : myForward(forward), 
      reverse(forward.makeReverser()) {}
  
  PairedConversionFunctors() 
      requires (std::default_initializable<SourceToTargetFunctor> && std::default_initializable<TargetToSourceFunctor>)
    : myForward(SourceToTargetFunctor()), 
      reverse(TargetToSourceFunctor()) {}
  
  Target operator()(const TSource& source) const {
    return (Target)source;
  }

 protected:
  const SourceToTargetFunctor myForward;

 public:
  const TargetToSourceFunctor reverse;
};

float fThree = 3.00001;
ConversionFunctor<float, int> converter;
auto i = converter(fThree);


template <DGtalTrace::concepts::CCell TSource>
class ConversionFunctor<TSource, typename TSource::Point> {
 public:
  typedef TSource Source;
  typedef typename TSource::Point Target;
  typedef Source::CellularGridSpace KSpace;
  
  ConversionFunctor(DGtal::ConstAlias<KSpace> ks) : myKPtr(&ks) {
    if (!myKPtr->isValid()) throw std::invalid_argument("ConversionFunctor constructor received invalid KSpace");
  }

  Target operator()(const Source& cell) const {
    Target result = myKPtr->uKCoords(cell);
    if (!myKPtr->uIsInside(result)) throw std::invalid_argument("Cell was outside KSpace.");
    return result;
  }

  ConversionFunctor<Target, Source> makeReverser() const {
    return ConversionFunctor<Target, Source>(*myKPtr);
  }

 private:
  const KSpace* myKPtr;
};

template <DGtalTrace::concepts::CPoint TSource, DGtalTrace::concepts::CCell TTarget>
class ConversionFunctor<TSource, TTarget> {
 public:
  typedef TSource Source;
  typedef typename TSource::Point Target;
  // typedef DGtal::Z2i::KSpace KSpace;
  typedef Source::CellularGridSpace KSpace;  

  ConversionFunctor(DGtal::ConstAlias<KSpace> ks) : myKPtr(&ks) {
    if (!myKPtr->isValid()) throw std::invalid_argument("ConversionFunctor constructor received invalid KSpace");
  }

  Target operator()(const Source& point) const {
    if (!myKPtr->uIsInside(point)) throw std::invalid_argument("Cell was outside KSpace.");
    return myKPtr->uCell(point);
  }

 private:
  const KSpace* myKPtr;
};


PairedConversionFunctors<float, int> p;
int i3hree = p(3);
float f3hree = p.reverse(i3hree);

} } // namespace DGtalTrace::functors


