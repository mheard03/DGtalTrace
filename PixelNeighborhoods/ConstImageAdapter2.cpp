#pragma once

#include <DGtal/base/BasicFunctors.h>
#include <DGtal/helpers/StdDefs.h>
#include <cassert>
#include <limits>
#if defined(ConstImageAdapter2_RECURSES)
#error Recursive header files inclusion detected in ConstImageAdapter2.h
#else // defined(ConstImageAdapter2_RECURSES)
/** Prevents recursive inclusion of headers. */
#define ConstImageAdapter2_RECURSES

#if !defined ConstImageAdapter2_h
/** Prevents repeated inclusion of headers. */
#define ConstImageAdapter2_h

//////////////////////////////////////////////////////////////////////////////
// Inclusions
#include "DGtal/base/Common.h"
#include "DGtal/base/ConceptUtils.h"
#include "DGtal/base/ConstAlias.h"
#include "DGtal/images/CImage.h"
#include "DGtal/kernel/domains/CDomain.h"
#include <cstdlib>
#include <iostream>
#include <type_traits>

#include "DGtal/images/DefaultConstImageRange.h"
//////////////////////////////////////////////////////////////////////////////

namespace DGtal {
/////////////////////////////////////////////////////////////////////////////
// Template class ConstImageAdapter
/**
 * Description of template class 'ConstImageAdapter2' <p>
 * \brief Aim: implements a const image adapter with a given domain
 * (i.e. a subdomain) and 2 functors : g for domain, f for accessing point
 * values.
 *
 * This class is (like Image class) a lightweight proxy on any models of
 * CConstImage. It uses a given Domain (i.e. a subdomain) but work directly (for
 * accessing process) thanks to an alias (i.e. a pointer) on the
 * original Image given in argument.
 *
 * ConstImageAdapter2 class is also a model of CConstImage.
 *
 * Caution :
 *  - the type of value of Point for the ConstImageAdapter2 Domain must
 * also be the same than the type of value of Point for the original
 * ImageContainer.
 *
 * @tparam TImageContainer an image container type (model of CConstImage).
 * @tparam TNewDomain a domain.
 * @tparam TFunctorD the functor g that transforms the domain into another one
 * @tparam TNewValue the type of value return by the functor f.
 * @tparam TFunctorV the functor f that transforms the value into another one
 * during reading process
 *
 * The values associated to accessing the point values are adapted
 * with a functor g and a functor f given at construction so that
 * operator() calls f(img(g(aPoint))), instead of calling directly
 * operator() of the underlying image img.
 *
 * Here is the construction of a simple ConstImageAdapter2 that
 * is a thresholded view of the initial scalar image:
 *
 * @snippet images/exampleConstImageAdapter.cpp
 * ConstImageAdapterForThresholderImage_creation
 *
 * NB: the underlying image as well as the 2 functors
 * are stored in the adapter as aliasing pointer
 * in order to avoid copies.
 * The pointed objects must exist and must not be deleted
 * during the use of the adapter
 */

template <typename TImageContainer, typename TFunctorD = DGtal::functors::Identity, typename TFunctorV = DGtal::functors::Identity>
class ConstImageAdapter2 {
 public:
  typedef ConstImageAdapter2<TImageContainer, TFunctorD, TFunctorV> Self;
  typedef TImageContainer ImageContainer;
  typedef typename ImageContainer::Domain Domain;
  typedef typename ImageContainer::Point Point;
  typedef typename ImageContainer::Value Value;
  typedef typename Z2i::KSpace KSpace;
  typedef typename Z2i::KPreSpace KPreSpace;
  typedef DefaultConstImageRange<Self> ConstRange;

  BOOST_CONCEPT_ASSERT((concepts::CConstImage<TImageContainer>));
  BOOST_CONCEPT_ASSERT((concepts::CUnaryFunctor<TFunctorD, Point, Point>));
  BOOST_CONCEPT_ASSERT((concepts::CUnaryFunctor<TFunctorV, Value, Value>));
  static_assert(std::is_default_constructible_v<Value>);
  static constexpr DGtal::functors::Identity fnIdentity;

 public:
  ConstImageAdapter2(ConstAlias<ImageContainer> anImage, ConstAlias<KSpace> aKSpace, ConstAlias<TFunctorD> aFD = fnIdentity, ConstAlias<TFunctorV> aFV = fnIdentity)
      : myImagePtr(&anImage), myKSpace(&aKSpace), myFD(&aFD), myFV(&aFV) {
    if constexpr (std::is_arithmetic_v<Value>) {
      defaultValue = 0;
    } else {
      defaultValue = Value();
    }

#ifdef DEBUG_VERBOSE
    trace.warning() << "ConstImageAdapter2 Ctor fromRef " << std::endl;
#endif
  }

  /**
   * Default copy constructor.
   * @param other the object to copy.
   */
  ConstImageAdapter2(const ConstImageAdapter2 &other) = default;

  /**
   * Assignment.
   * @param other the object to copy.
   * @return a reference on 'this'.
   */
  ConstImageAdapter2 &operator=(const ConstImageAdapter2 &other) {
#ifdef DEBUG_VERBOSE
    trace.warning() << "ConstImageAdapter2 assignment " << std::endl;
#endif
    if (&other != this) {
      myImagePtr = other.myImagePtr;
      myKSpace = other.myKSpace;
      myFD = other.myFD;
      myFV = other.myFV;
      defaultValue = other.defaultValue;
    }
    return *this;
  }

  /**
   * Destructor.
   * Does nothing
   */
  ~ConstImageAdapter2() {}

  // ----------------------- Interface --------------------------------------
public:
  /////////////////// Domains //////////////////

  /**
   * Returns a reference to the underlying image domain.
   *
   * @return a reference to the domain.
   */
  const Domain &domain() const { return (*myImagePtr->domain()); }

  /**
   * Returns the range of the underlying image
   * to iterate over its values
   *
   * @return a range.
   */
  ConstRange constRange() const { return ConstRange(*this); }

  /////////////////// Accessors //////////////////

  /**
   * Get the value of an image at a given position given
   * by a Point.
   *
   * @pre the point must be in the domain
   *
   * @param aPoint the point.
   * @return the value at aPoint.
   */
  Value operator()(const Point &aPoint) const {
    typename TImageContainer::Point point = myFD->operator()(point);
    if (myKSpace->uIsInside(aPoint)) {
      point = (myKSpace->isAnyDimensionPeriodic()) ? myKSpace->uCoords(myKSpace->uSpel(aPoint)) : aPoint;
      if (this->domain().isInside(point)) {      
        if (myImagePtr->domain().isInside(point)) {
          return myFV->operator()(myImagePtr->operator()(point));
        }
      }
    }
    return defaultValue;
  }

  /////////////////// API //////////////////

  /**
   * Writes/Displays the object on an output stream.
   * @param out the output stream where the object is written.
   */
  void selfDisplay(std::ostream &out) const { out << "[ConstImageAdapter2] " << (*myImagePtr); }

  /**
   * Checks the validity/consistency of the object.
   * @return 'true' if the object is valid, 'false' otherwise.
   */
  bool isValid() const { return (myImagePtr->isValid()); }

  /**
   * @return the style name used for drawing this object.
   */
  std::string className() const { return "ConstImageAdapter2"; }

  /**
   * Returns the pointer on the Image container data.
   * @return a const ImagePtr.
   */
  const ImageContainer *getPointer() const { return myImagePtr; }
  
  /**
   * Allows to define a default value returned when point
   * transformed by domain functor does not belongs to
   * image domain.
   */
  void setDefaultValue(Value aValue) { defaultValue = aValue; }

  Value getDefaultValue() const { return defaultValue; }

  // ------------------------- Protected Datas ------------------------------
private:
  ConstImageAdapter2() {
#ifdef DEBUG_VERBOSE
    trace.warning() << "ConstImageAdapter2 Ctor default " << std::endl;
#endif
  }

  // ------------------------- Private Datas --------------------------------
protected:
  const ImageContainer *myImagePtr;
  const KSpace *myKSpace;
  const TFunctorD *myFD;
  const TFunctorV *myFV;

  /**
   *  Default value returned when point transformed by image functor does not
   * belongs to image. Initial value is 0.
   */
  Value defaultValue;

}; // end of class ConstImageAdapter

/**
 * Overloads 'operator<<' for displaying objects of class 'ConstImageAdapter2'.
 * @param out the output stream where the object is written.
 * @param object the object of class 'ConstImageAdapter2' to write.
 * @return the output stream after the writing.
 */
template <typename TImageContainer, typename TFunctorD, typename TFunctorV>
std::ostream &
operator<<(std::ostream &out, const ConstImageAdapter2<TImageContainer, TFunctorD, TFunctorV> &object) {
  object.selfDisplay(out);
  return out;
}

} // namespace DGtal

///////////////////////////////////////////////////////////////////////////////
// Includes inline functions.
/*
template <typename TImageContainer, typename TFunctorD, typename TFunctorV>
inline void
DGtal::ConstImageAdapter2<TImageContainer, TFunctorD, TFunctorV>::selfDisplay(std::ostream &out) const {
  out << "[ConstImageAdapter2] " << (*myImagePtr);
}
*/
/*
template <typename TImageContainer, typename TFunctorD, typename TFunctorV>
inline std::string
DGtal::ConstImageAdapter2<TImageContainer, TFunctorD, TFunctorV>::className() const {
  return "ConstImageAdapter2";
}
*/
///////////////////////////////////////////////////////////////////////////////
// Implementation of inline functions                                        //
/*
template <typename TImageContainer, typename TFunctorD, typename TFunctorV>
inline std::ostream &DGtal::operator<<(
    std::ostream &out,
    const ConstImageAdapter2<TImageContainer, TFunctorD, TFunctorV> &object) {
  object.selfDisplay(out);
  return out;
}
*/
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

#endif // !defined ConstImageAdapter2_h

#undef ConstImageAdapter2_RECURSES
#endif // else defined(ConstImageAdapter2_RECURSES)
