#pragma once
#include <DGtal/base/Common.h>
#include <array>
#include <DGtal/helpers/StdDefs.h>
#include <DGtal/base/Trace.h>
#include <DGtal/kernel/domains/CDomain.h>
#include "traceHelper.cpp"

typedef DGtal::Z2i::Domain Domain;

template <typename I>
struct CConstImage : CTrivialConstImage<I> {

public:

  //Inner types
  typedef typename I::Domain Domain;
  BOOST_CONCEPT_ASSERT((concepts::CDomain<Domain>));

