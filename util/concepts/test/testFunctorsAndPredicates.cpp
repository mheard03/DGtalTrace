#pragma once
#include <concepts>
#include <DGtal/base/Common.h>
#include <DGtal/helpers/StdDefs.h>
#include <DGtal/images/ImageSelector.h>
#include "../dgtal/CUnaryFunctor.cpp"
#include "../dgtal/CPointFunctor.cpp"
#include "../dgtal/CPredicate.cpp"
#include "../dgtal/CPointPredicate.cpp"


using namespace DGtalTrace::concepts;
using namespace DGtal;
using namespace DGtal::functors;

// PointPredicate concepts
namespace {

typedef ImageSelector<Z3i::Domain, int>::Type IntImage;
using IntPredicate = std::function<bool(int)>;
typedef ImageSelector<Z3i::Domain, float>::Type FloatImage;
using FloatPredicate = std::function<bool(float)>;

// PointFunctorPredicate
typedef PointFunctorPredicate<IntImage, IntPredicate> PointPredicate1;
static_assert(( CPointPredicate< PointPredicate1 > ));

typedef PointFunctorPredicate<FloatImage, FloatPredicate> PointPredicate2;
static_assert (( CPointPredicate< PointPredicate2 > ));

// Binary PointPredicate
typedef std::logical_and<bool> BinaryFunctor;
static_assert (( CPointPredicate< BinaryPointPredicate<PointPredicate1, PointPredicate2, BinaryFunctor> > ));

// NotPointPredicate
static_assert(( CPointPredicate< NotPointPredicate<PointPredicate1> > ));

typedef typename IntImage::Point Point;
static_assert(( CPointPredicate< EqualPointPredicate<Point> > ));

}




namespace {
static_assert(CUnaryFunctor<UnaryMinus<int>, int, int> == true);
static_assert(CUnaryFunctor<UnaryMinus<int>, int, std::string> == false);
} // namespace
