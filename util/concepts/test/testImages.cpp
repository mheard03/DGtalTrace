#pragma once
#include <DGtal/base/Common.h>
#include <DGtal/base/LabelledMap.h>
#include <DGtal/images/ImageSelector.h>
#include <DGtal/kernel/SpaceND.h>
#include <DGtal/kernel/domains/HyperRectDomain.h>
#include <DGtal/images/ImageContainerBySTLVector.h>
#include "../dgtal/CConstSinglePassRange.cpp"
#include "../dgtal/CDomain.cpp"
#include "../dgtal/CTrivialConstImage.cpp"
#include "../dgtal/CTrivialImage.cpp"
#include "../dgtal/CConstImage.cpp"


using namespace DGtalTrace::concepts;
using namespace DGtal;

// CConstSinglePassRange
namespace {
  typedef DGtal::HyperRectDomain<DGtal::SpaceND<2>> TDomain;
  static_assert(( CConstSinglePassRange<TDomain> ));  
  static_assert(( CDomain<TDomain> ));

  typedef DGtal::ImageContainerBySTLVector<TDomain, int> TImage;
  static_assert(( CTrivialConstImage<TImage> ));
  static_assert(( CTrivialImage<TImage> ));
  static_assert(( CConstImage<TImage> ));



} // namespace
