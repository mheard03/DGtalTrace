#pragma once
#include <cmath>
#include <cassert>
#include <DGtal/base/Common.h>
#include <DGtal/helpers/StdDefs.h>
#include <DGtal/kernel/PointVector.h>
#include <DGtal/kernel/SpaceND.h>
#include <DGtal/kernel/ArithmeticConversionTraits.h>
#include <DGtal/kernel/domains/HyperRectDomain.h>
#include <DGtal/kernel/BasicPointFunctors.h>
#include <DGtal/kernel/PointFunctorHolder.h>
#include "KCellConstraints.cpp"
#include <type_traits>

template <typename TDomain>
class DomainScalerFunctor {
 public:
  // typedef TDomain Domain;
  typedef typename DGtal::Z2i::Domain Domain;
  typedef typename Domain::Point Point;
  static constexpr DGtal::Dimension dimension = Domain::dimension;

  DomainScalerFunctor(DGtal::ConstAlias<Domain> aSourceDomain, uint scale, bool scaleUpwards, const Point &targetDomainLower = Point::zero) : 
      mySourceDomainPtr(&aSourceDomain),
      myScale(scale),
      myScaleUpwards(scaleUpwards),
      myTargetLower(targetDomainLower) {
    if (aSourceDomain->isEmpty()) [[unlikely]]
      throw std::invalid_argument("Domain can't be empty");

    if (scale == 0) [[unlikely]] 
      throw std::invalid_argument("Scale can't be 0");

    if (!scaleUpwards) {
      auto dims = mySourceDomainPtr->upperBound() - mySourceDomainPtr->lowerBound();
      for (auto i = 0; i < Domain::dimension; i++) {
        if (dims[i] % scale != 0) [[unlikely]]
          throw std::invalid_argument("Source domain extent must be evenly divisible by " + std::to_string(scale) + " on dimension " + std::to_string(i) + " (got size " + std::to_string(dims[i]) + ")");
      }
    }
  }

  inline Point operator()(const Point& sourcePoint) const {
    double appliedScale = myScaleUpwards ? myScale : (1.0 / myScale);
    DGtal::PointVector<dimension, double> targetPoint(sourcePoint);
    
    targetPoint -= mySourceDomainPtr->lowerBound();
    targetPoint *= appliedScale;
    targetPoint += myTargetLower;
    return Point(targetPoint);
  }

 private:
  const Domain *mySourceDomainPtr;
  const uint myScale;
  const bool myScaleUpwards = false;
  const Point myTargetLower = Point::zero;
};


template <typename TDomain>
class ScaledDomainPair {
 public:
  // typedef TDomain Domain
  typedef DGtal::Z2i::Domain Domain;
  typedef typename Domain::Point Point;
  typedef DomainScalerFunctor<Domain> Functor;

  ScaledDomainPair(const Point &sourceDomainLower, const Point &sourceDomainUpper, double targetScale, const Point &targetDomainLower = Point::zero) :
    mySourceDomain(sourceDomainLower, sourceDomainUpper),
    myFuncSourceToTarget(Functor(&mySourceDomain, targetScale, false, targetDomainLower)),
    myTargetDomain(myFuncSourceToTarget(sourceDomainLower), myFuncSourceToTarget(sourceDomainUpper)),
    myFuncTargetToSource(Functor(&myTargetDomain, targetScale, true, sourceDomainLower))
  {}

  const Domain& sourceDomain() const { return mySourceDomain; }
  const Domain& targetDomain() const { return myTargetDomain; }
  inline const Point toTarget(Point p) const { return myFuncSourceToTarget(p); }
  inline const Point toSource(Point p) const { return myFuncTargetToSource(p); }

  template <typename TCell> requires CAnyCell<TCell>
  inline const Point toTarget(TCell c) const { return toTarget(c.preCell().coordinates()); }
  template <typename TCell> requires CAnyCell<TCell>
  inline const Point toSource(TCell c) const { return toSource(c.preCell().coordinates()); }

 private:
  const Domain mySourceDomain;
  const DomainScalerFunctor<Domain> myFuncSourceToTarget;
  const Domain myTargetDomain;  
  const DomainScalerFunctor<Domain> myFuncTargetToSource;
};