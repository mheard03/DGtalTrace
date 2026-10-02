#pragma once
#include <DGtal/base/CLabel.h>
#include <DGtal/base/Common.h>
#include <DGtal/base/SimpleRandomAccessConstRangeFromPoint.h>
#include <DGtal/base/SimpleRandomAccessRangeFromPoint.h>
#include <DGtal/kernel/NumberTraits.h>
#include <DGtal/kernel/SpaceND.h>
#include <DGtal/kernel/domains/CDomain.h>
#include <DGtal/kernel/domains/HyperRectDomain.h>
#include <cstddef>
#include <iostream>
#include <vector>
#include "../concepts/dgtal/images/CImage.cpp"

namespace DGtalTrace {

namespace detail {

}

template <typename TDomain, typename TValue>
class ImageContainerByCompressedVector {
  struct Span {
    TValue value;
    size_t index;
    Span(const TValue aValue, const size_t anIndex) : value(aValue), index(anIndex) {}
  };

public:
  typedef ImageContainerByCompressedVector<TDomain, TValue> Self;
  typedef std::vector<TValue> Parent;
  typedef Parent Container;

  /// domain
  BOOST_CONCEPT_ASSERT((concepts::CDomain<TDomain>));
  typedef TDomain Domain;
  typedef typename Domain::Point Point;
  typedef typename Domain::Vector Vector;
  typedef typename Domain::Integer Integer;
  typedef typename Domain::Size Size;
  typedef typename Domain::Dimension Dimension;
  typedef Point Vertex;

  BOOST_STATIC_CONSTANT(Dimension, dimension = Domain::Space::dimension);

  /// domain should be rectangular
  BOOST_STATIC_ASSERT(
      (boost::is_same<Domain, HyperRectDomain<typename Domain::Space>>::value));

  /// range of values
  BOOST_CONCEPT_ASSERT((concepts::CLabel<TValue>));
  typedef TValue Value;

  /////////////////// Data members //////////////////

private:
  /// Image domain
  Domain myDomain;

  /// Domain extent (stored for linearization efficiency)
  Vector myExtent;

  /////////////////// standard services //////////////////

public:
  /**
   * Constructor from a Domain
   *
   * @param aDomain the image domain.
   *
   */
  ImageContainerBySTLVector(const Domain &aDomain);

  /**
   * Copy constructor
   *
   * @param other the object to copy.
   *
   */
  ImageContainerBySTLVector(const ImageContainerBySTLVector &other);

  /**
   * Assignment operator
   *
   * @param other the object to copy.
   *
   * @return a reference on *this
   */
  ImageContainerBySTLVector &operator=(const ImageContainerBySTLVector &other);

  /**
   * Destructor.
   *
   */
  ~ImageContainerBySTLVector();

  /////////////////// Interface //////////////////

  /**
   * Get the value of an image at a given position given
   * by a Point.
   *
   * @pre the point must be in the domain
   *
   * @param aPoint the point.
   * @return the value at aPoint.
   */
  Value operator()(const Point &aPoint) const;

  /**
   * Set a value on an Image at a position specified by a Point.
   *
   * @pre @c it must be a point in the image domain.
   *
   * @param aPoint the point.
   * @param aValue the value.
   */
  void setValue(const Point &aPoint, const Value &aValue);

  /**
   * @return the domain associated to the image.
   */
  const Domain &domain() const;

  /**
   * @return the domain extension of the image.
   */
  const Vector &extent() const;

  /**
   * Translate the underlying domain by @a aShift
   * @param aShift any vector
   */
  void translateDomain(const Vector &aShift);

  /**
   * Writes/Displays the object on an output stream.
   * @param out the output stream where the object is written.
   */
  void selfDisplay(std::ostream &out) const;

  /**
   * @return the validity of the Image
   */
  bool isValid() const;

  // ------------- realization CDrawableWithBoard2D --------------------

  /**
   * @return the style name used for drawing this object.
   */
  std::string className() const;

  /////////////////////////// Iterators ////////////////////
  // built-in iterators
  typedef typename std::vector<Value>::iterator Iterator;
  typedef typename std::vector<Value>::const_iterator ConstIterator;
  typedef typename std::vector<Value>::reverse_iterator ReverseIterator;
  typedef
      typename std::vector<Value>::const_reverse_iterator ConstReverseIterator;
  typedef typename std::vector<Value>::difference_type Difference;

  typedef typename std::vector<Value>::iterator OutputIterator;
  typedef typename std::vector<Value>::reverse_iterator ReverseOutputIterator;

  /////////////////////////// Ranges  /////////////////////
  typedef SimpleRandomAccessConstRangeFromPoint<ConstIterator,
                                                DistanceFunctorFromPoint<Self>>
      ConstRange;
  typedef SimpleRandomAccessRangeFromPoint<ConstIterator, Iterator,
                                           DistanceFunctorFromPoint<Self>>
      Range;

  /**
   * @return the range providing begin and end
   * iterators to scan the values of image.
   */
  ConstRange constRange() const;

  /**
   * @return the range providing begin and end
   * iterators to scan the values of image.
   */
  Range range();

  /**
   * Give access to the underlying container.
   * @return a (might be const) reference to the container.
   */
  const Container &container() const { return static_cast<Parent>(*this); };
  /**
   * Give access to the underlying container.
   * @return a (might be const) reference to the container.
   */
  Container &container() { return static_cast<Parent>(*this); };

  /////////////////////////// Custom Iterator ///////////////
  /**
   * Specific SpanIterator on ImageContainerBySTLVector.
   *
   */

  class SpanIterator {

    friend class ImageContainerBySTLVector<Domain, Value>;

  public:
    ///\todo construct a RANDOM-ACCESS iterator
    typedef std::bidirectional_iterator_tag iterator_category;
    typedef Value value_type;
    typedef ptrdiff_t difference_type;
    typedef Value *pointer;
    typedef Value &reference;

    /**
     * Constructor.
     *
     * @param p starting point of the SpanIterator
     * @param aDim specifies the dimension along which the iterator will iterate
     * @param aMap pointer to the imageContainer
     */
    SpanIterator(const Point &p, const Dimension aDim,
                 ImageContainerBySTLVector<Domain, Value> *aMap)
        : myMap(aMap), myDimension(aDim) {
      myPos = aMap->linearized(p);

      // We compute the myShift quantity
      myShift = 1;

      for (Dimension k = 0; k < myDimension; k++)
        myShift *= aMap->myExtent[k];
    }

    /**
     * Set a value at a SpanIterator position.
     *
     * @param aVal the value to set.
     */
    inline void setValue(const Value aVal) { (*myMap)[myPos] = aVal; }

    /**
     * operator* on SpanIterators.
     *
     * @return the value associated to the current position.
     */
    inline const Value &operator*() { return (*myMap)[myPos]; }

    /**
     * Operator ==.
     *
     * @return true if this and it are equals.
     */
    inline bool operator==(const SpanIterator &it) const {
      return (myPos == it.myPos);
    }

    /**
     * Operator !=
     *
     * @return true if this and it are different.
     */
    inline bool operator!=(const SpanIterator &it) const {
      return (myPos != it.myPos);
    }

    /**
     * Implements the next() method: we move on step forward.
     *
     **/
    inline void next() { myPos += myShift; }

    /**
     * Implements the prev() method: we move on step backward.
     *
     **/
    inline void prev() {
      ASSERT((long int)myPos - myShift > 0);
      myPos -= myShift;
    }

    /**
     * Operator ++ (++it)
     *
     */
    inline SpanIterator &operator++() {
      this->next();
      return *this;
    }

    /**
     * Operator ++ (it++)
     *
     */
    inline SpanIterator &operator++(int) {
      SpanIterator tmp = *this;
      ++*this;
      return tmp;
    }

    /**
     * Operator -- (--it)
     *
     */
    inline SpanIterator &operator--() {
      this->prev();
      return *this;
    }

    /**
     * Operator -- (it--)
     *
     */
    inline SpanIterator &operator--(int) {
      SpanIterator tmp = *this;
      --*this;
      return tmp;
    }

  private:
    /// Current Point in the domain
    Size myPos;

    /// Copy of the underlying images
    ImageContainerBySTLVector<Domain, Value> *myMap;

    /// Dimension on which the iterator must iterate
    Dimension myDimension;

    /// Padding variable
    Size myShift;
  };

  /**
   * Set a value on an Image at a position specified by an SpanIterator.
   *
   * @param it  iterator on the location.
   * @param aValue the value.
   */
  void setValue(SpanIterator &it, const Value &aValue) { it.setValue(aValue); }

  /**
   * Create a begin() SpanIterator at a given position in a given
   * direction.
   *
   * @param aPoint the starting point of the SpanIterator.
   * @param aDimension the dimension on which the iterator iterates.
   *
   * @return a SpanIterator
   */
  SpanIterator spanBegin(const Point &aPoint, const Dimension aDimension) {
    return SpanIterator(aPoint, aDimension, this);
  }

  /**
   * Create an end() SpanIterator at a given position in a given
   * direction.
   *
   * @param aPoint a point belonging to the current image dimension (not
   * necessarily the point used in the span_begin() method.
   * @param aDimension the dimension on which the iterator iterates.
   *
   * @return a SpanIterator
   */
  SpanIterator spanEnd(const Point &aPoint, const Dimension aDimension) {
    Point tmp = aPoint;
    tmp[aDimension] = myDomain.upperBound()[aDimension] + 1;
    return SpanIterator(tmp, aDimension, this);
  }

  /**
   * Returns the value of the image at a given SpanIterator position.
   *
   * @param it position given by a SpanIterator.
   * @return an object of type Value.
   */
  Value getValue(SpanIterator &it) { return (*it); };

  /**
   *  Linearized a point and return the vector position.
   * @param aPoint the point to convert to an index
   * @return the index of @a aPoint in the container
   */
  Size linearized(const Point &aPoint) const;
};

/**
 * Overloads 'operator<<' for displaying objects of class 'Image'.
 * @param out the output stream where the object is written.
 * @param object the object of class 'Image' to write.
 * @return the output stream after the writing.
 */
template <typename Domain, typename V>
inline std::ostream &
operator<<(std::ostream &out,
           const ImageContainerBySTLVector<Domain, V> &object) {
  object.selfDisplay(out);
  return out;
}

} // namespace DGtalTrace

///////////////////////////////////////////////////////////////////////////////
// Includes inline functions
#include "DGtal/images/ImageContainerBySTLVector.ih"

//                                                                           //
///////////////////////////////////////////////////////////////////////////////

#endif // !defined ImageContainerBySTLVector_h

#undef ImageContainerBySTLVector_RECURSES
#endif // else defined(ImageContainerBySTLVector_RECURSES)
