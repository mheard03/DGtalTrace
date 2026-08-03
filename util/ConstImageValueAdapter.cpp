#pragma once
#include "../concepts/dgtal/CDomain.cpp"
#include "../concepts/CIsOptional.cpp"
#include "../concepts/dgtal/images/CConstImage.cpp"
#include "DGtal/images/DefaultConstImageRange.h"
#include "Globals.cpp"
#include <DGtal/base/BasicFunctors.h>
#include <DGtal/base/Common.h>
#include <DGtal/base/ConstAlias.h>
#include <iostream>
#include <optional>
#include <type_traits>

namespace DGtalTrace {

namespace {

struct OptIdentityFunctor {
  template <typename T>
  inline std::optional<T> operator()(const T &value) const {
    return std::optional<T>(value);
  }
};

template <typename OldValue, typename NewValue>
using DefaultValueFunctorType = std::conditional_t<
    std::same_as<OldValue, NewValue>, DGtal::functors::Identity,
    std::conditional_t<std::same_as<std::optional<OldValue>, NewValue>,
                       OptIdentityFunctor, void>>;

template <typename OldValue, typename NewValue>
const DefaultValueFunctorType<OldValue, NewValue> defaultValueFunctor;

} // end anonymous namespace

/**
 * Description of template class 'ConstImageAdapter' <p>
 * \brief Aim: implements a const image adapter with a given domain
 * (i.e. a subdomain) and 2 functors : g for domain, f for accessing point
 * values.
 *
 * This class is (like Image class) a lightweight proxy on any models of
 * CConstImage. It uses a given Domain (i.e. a subdomain) but work directly (for
 * accessing process) thanks to an alias (i.e. a pointer) on the
 * original Image given in argument.
 *
 * ConstImageAdapter class is also a model of CConstImage.
 *
 * Caution :
 *  - the type of value of Point for the ConstImageAdapter Domain must
 * also be the same than the type of value of Point for the original
 * ImageContainer.
 *
 * @tparam TImageContainer an image container type (model of CConstImage).
 * @tparam TNewValue the type of value return by the functor f.
 * @tparam TFunctorV the functor f that transforms the value into another one
 * during reading process
 *
 * The values associated to accessing the point values are adapted
 * with a functor g and a functor f given at construction so that
 * operator() calls f(img(g(aPoint))), instead of calling directly
 * operator() of the underlying image img.
 *
 * Here is the construction of a simple ConstImageAdapter that
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
template <concepts::CConstImage TImageContainer, concepts::CLabel TNewValue,
          typename TFunctorV = DefaultValueFunctorType<typename TImageContainer::Value, TNewValue>>
requires concepts::CIsFalse<std::is_void<TFunctorV>>
class ConstImageValueAdapter {
public:
  typedef ConstImageValueAdapter<TImageContainer, TNewValue, TFunctorV> Self;
  typedef TNewValue Value;

  /// Types copied from the container
  typedef TImageContainer ImageContainer;
  typedef ImageContainer::Point Point;
  typedef ImageContainer::Domain Domain;
  typedef DGtal::DefaultConstImageRange<Self> ConstRange;

private:
  typedef DefaultValueFunctorType<typename TImageContainer::Value, TNewValue> DVFType;

  // ----------------------- Standard services ------------------------------

public:
  ConstImageValueAdapter(DGtal::ConstAlias<ImageContainer> anImage,
                         DGtal::ConstAlias<TFunctorV> aFV)
      : myImagePtr(&anImage), myFV(&aFV) {
  }

  ConstImageValueAdapter(DGtal::ConstAlias<ImageContainer> anImage)
    requires std::same_as<TFunctorV, DVFType>
      : myImagePtr(&anImage), myFV(&defaultValueFunctor<typename TImageContainer::Value, TNewValue>) {}

  ConstImageValueAdapter(const ConstImageValueAdapter &other) = default;

  ConstImageValueAdapter &operator=(const ConstImageValueAdapter &other) {
    if (&other != this) {
      myImagePtr = other.myImagePtr;
      myFV = other.myFV;
      defaultValue = other.defaultValue;
    }
    return *this;
  }

  // ----------------------- Interface --------------------------------------
public:
  /////////////////// Domains //////////////////

  /**
   * Returns a reference to the underlying image domain.
   * @return a reference to the domain.
   */
  const Domain &domain() const { return myImagePtr->domain(); }

  /**
   * Returns the range of the underlying image to iterate over its values
   * @return a range.
   */
  ConstRange constRange() const { return myImagePtr->constRange(); }

  /////////////////// Accessors //////////////////

  /**
   * Get the value of an image at a given position given by a Point.
   * @pre the point must be in the domain
   * @param aPoint the point.
   * @return the value at aPoint.
   */
  Value operator()(const Point &aPoint) const {
    if (myImagePtr->domain().isInside(aPoint))
      return myFV->operator()(myImagePtr->operator()(aPoint));
    else
      return defaultValue;
  }

  /////////////////// API //////////////////

  /**
   * Writes/Displays the object on an output stream.
   * @param out the output stream where the object is written.
   */
  void selfDisplay(std::ostream &out) const {
    out << "[ConstImageValueAdapter " << (*myImagePtr) << "]";
  }

  /**
   * Checks the validity/consistency of the object.
   * @return 'true' if the object is valid, 'false' otherwise.
   */
  bool isValid() const { return myImagePtr->isValid(); }

  /**
   * @return the style name used for drawing this object.
   */
  std::string className() const { return "ConstImageValueAdapter"; }

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
  void setDefaultValue(const Value &aValue) { defaultValue = aValue; }

  Value getDefaultValue() const { return defaultValue; }

private:
  ConstImageValueAdapter() {}
  const ImageContainer *myImagePtr;
  const TFunctorV *myFV;
  Value defaultValue;

}; // end of class ConstImageAdapter

/**
 * Overloads 'operator<<' for displaying objects of class 'ConstImageAdapter'.
 * @param out the output stream where the object is written.
 * @param object the object of class 'ConstImageAdapter' to write.
 * @return the output stream after the writing.
 */
template <typename TImageContainer, typename TNewDomain, typename TFunctorD,
          typename TNewValue, typename TFunctorV>
std::ostream &
operator<<(std::ostream &out,
           const ConstImageValueAdapter<TImageContainer, TNewValue, TFunctorV>
               &object) {
  object.selfDisplay(out);
  return out;
}

} // namespace DGtalTrace