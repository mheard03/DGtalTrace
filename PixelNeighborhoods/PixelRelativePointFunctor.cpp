#pragma once
#include <DGtal/base/Common.h>
#include <DGtal/base/ConceptUtils.h>
#include "DGtal/base/ConstAlias.h"
#include <DGtal/helpers/StdDefs.h>
#include <cassert>
#include <DGtal/images/CImage.h>
#include "../util/traceHelper.cpp"
#include "../CEVec.cpp"
#include "CWAnglesEnum.hpp"
#include "DirectionsEnum.hpp"
#include "DirectionsOffsets.cpp"

#include <DGtal/images/Image.h>
#include <DGtal/images/ImageSelector.h>
#include <numeric>

namespace DGtalTrace { namespace Offsets { 

template <typename TImageContainer>
struct PixelRelativePointFunctor
{
  typedef PixelRelativePointFunctor<TImageContainer> Self;
  typedef DGtal::Z2i::KSpace KSpace;
  typedef DGtal::Z2i::KPreSpace KPreSpace;
  typedef TImageContainer ImageContainer;
  typedef typename ImageContainer::Domain Domain;
  typedef DGtal::Z2i::Point Point;
  typedef typename ImageContainer::Value Value;
  typedef std::function<CEVec2(const CEVec2&)> FnRotatePtr; // CEVec2 (*FnRotatePtr)(const CEVec2&);
  BOOST_CONCEPT_ASSERT((DGtal::concepts::CConstImage<TImageContainer>));
  static_assert(Domain::dimension == 2);

  PixelRelativePointFunctor(DGtal::ConstAlias<KSpace> aKSpace, const Point aCenterPoint = Point(0, 0), const CWAngles aAngle = CWAngles::Rotate0) : myKSpace(&aKSpace), myCenterPoint(aCenterPoint), myAngle(aAngle) {
    setAngle(aAngle);
  };

  const bool isInside(const Point& lowerOffset, const Point& upperOffset) {
    if (myKSpace->isSpacePeriodic()) return true;

    std::array<CEVec2, 4> corners;
    if (static_cast<size_t>(myAngle) % 180 == 0) {
      corners = { lowerOffset, upperOffset };
    }
    else {
      corners = { CEVec2(lowerOffset[0], upperOffset[1]), CEVec2(upperOffset[0], lowerOffset[1]) };
    }
    return isInside(corners[0]) && isInside(corners[1]);
  }

  inline const bool isInside(const Point& offset) const {
    return myKSpace->uIsInside(KPreSpace::uSpel(this->operator()(offset)));
  }

  inline Point operator()(const Point& offset) const {
    return myRotate(offset) + myCenterPoint;
  }

  const Value centerPoint() const {
    return myCenterPoint;
  }
  
  void setCenterPoint(const Point& centerPoint) {
    myCenterPoint = centerPoint;
  }

  const CWAngles angle() const {
    return myAngle;
  }
  
  void setAngle(const CWAngles angle) {
    myAngle = angle;
    switch (angle) {
      #define X(enum, num, ...) case(CWAngles::enum): myRotate = &Self:: rot ## num ; break;
      FOR_ANGLES(X)
      #undef X
    }
  }

 private:
  inline static CEVec2 rot0(const CEVec2& offset) { return CEVec2(offset); }
  inline static CEVec2 rot90(const CEVec2& offset) { return CEVec2(-offset[1], offset[0]); }
  inline static CEVec2 rot180(const CEVec2& offset) { return CEVec2(-offset[0], -offset[1]); }
  inline static CEVec2 rot270(const CEVec2& offset) { return CEVec2(offset[1], -offset[0]); }

  const KSpace* myKSpace;
  Point myCenterPoint;
  CWAngles myAngle;
  FnRotatePtr myRotate = &Self::rot0;
};


} } // end namespace