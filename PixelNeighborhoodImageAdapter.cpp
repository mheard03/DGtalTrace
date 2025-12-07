#pragma once
#include <DGtal/base/Common.h>
#include <boost/move/detail/meta_utils.hpp>
#include <cstdarg>
#include <algorithm>
#include <array>
#include <cstdint>
#include <cstdlib>
#include <deque>
#include <functional>
#include <initializer_list>
#include <limits>
#include <memory>
#include <string_view>
#include <type_traits>
#include <utility>
#include <unistd.h>
#include <vector>
#include <numeric>
#include <DGtal/helpers/StdDefs.h>
#include <DGtal/base/Trace.h>
#include <DGtal/base/ConstAlias.h>
#include "traceHelper.cpp"
#include "CEVec.cpp"

#include <DGtal/images/Image.h>
#include <DGtal/images/ImageSelector.h>
#include <DGtal/base/BasicFunctors.h>
#include "ConstImageAdapter2.cpp"

enum class Angles : size_t { Rotate0, Rotate90, Rotate180, Rotate270 };

#define FOR_ANGLES_INLINE(X, ...) X(Angles::Rotate0, 0, __VA_ARGS__), X(Angles::Rotate90, 90, __VA_ARGS__), X(Angles::Rotate180, 180, __VA_ARGS__), X(Angles::Rotate270, 270, __VA_ARGS__)
#define FOR_ANGLES(X, ...) X(Angles::Rotate0, 0, __VA_ARGS__) \
  X(Angles::Rotate90, 90, __VA_ARGS__) \
  X(Angles::Rotate180, 180, __VA_ARGS__) \
  X(Angles::Rotate270, 270, __VA_ARGS__)

#define FOR_DIRECTIONS_INLINE(X, ...) X(nw, __VA_ARGS__), X(n, __VA_ARGS__), X(ne, __VA_ARGS__), X(w, __VA_ARGS__), X(c, __VA_ARGS__), X(e, __VA_ARGS__), X(sw, __VA_ARGS__), X(s, __VA_ARGS__), X(se, __VA_ARGS__)
#define FOR_DIRECTIONS(X, ...) X(nw, __VA_ARGS__) \
  X(n, __VA_ARGS__) \
  X(ne, __VA_ARGS__) \
  X(w, __VA_ARGS__) \
  X(c, __VA_ARGS__) \
  X(e, __VA_ARGS__) \
  X(sw, __VA_ARGS__) \
  X(s, __VA_ARGS__) \
  X(se, __VA_ARGS__)

template <Angles CWAngle>
inline constexpr const CEVec2 getRotated(const CEVec2::Container& coords) {
  const auto [x, y] = coords;
  if constexpr (CWAngle == Angles::Rotate0) return CEVec2(x, y);
  if constexpr (CWAngle == Angles::Rotate90) return CEVec2(-y, x);
  if constexpr (CWAngle == Angles::Rotate180) return CEVec2(-x, -y);
  if constexpr (CWAngle == Angles::Rotate270) return CEVec2(y, -x);
}

template <Angles CWAngle>
struct Directions {
  Directions() = delete;

#define dir(name, dx, dy) \
  static constexpr CEVec2 name = getRotated<CWAngle>({dx, dy}); 

  dir(nw, -1, -1);
  dir(n,  +0, -1);
  dir(ne, +1, -1);
  dir(w,  -1, +0);
  dir(c,  +0, +0);
  dir(e,  +1, +0);
  dir(sw, -1, +1);
  dir(s,  +0, +1);
  dir(se, +1, +1);
  
  #undef dir
};

/*
inline constexpr const CEVec2 getRotated(const Angles CWAngle, const CEVec2::Container& coords) {
  const auto [x, y] = coords;
  if (CWAngle == Angles::Rotate0) return CEVec2(x, y);
  if (CWAngle == Angles::Rotate90) return CEVec2(-y, x);
  if (CWAngle == Angles::Rotate180) return CEVec2(-x, -y);
  if (CWAngle == Angles::Rotate270) return CEVec2(y, -x);
}

struct Directions {
 private:
  static constexpr const CEVec2 o_nw = CEVec2(-1, -1);
  static constexpr const CEVec2 o_n =  CEVec2(+0, -1);
  static constexpr const CEVec2 o_ne = CEVec2(+1, -1);
  static constexpr const CEVec2 o_w =  CEVec2(-1, +0);
  static constexpr const CEVec2 o_c =  CEVec2(+0, +0);
  static constexpr const CEVec2 o_e =  CEVec2(+1, +0);
  static constexpr const CEVec2 o_sw = CEVec2(-1, +1);
  static constexpr const CEVec2 o_s =  CEVec2(+0, +1);
  static constexpr const CEVec2 o_se = CEVec2(+1, +1);

 public:
#define dir_field(name, ...) const CEVec2 name;
FOR_DIRECTIONS(dir_field);
#undef dir_field

#define dir_init(name, ...) name(getRotated(angle, o_ ## name))
Directions(const Angles angle) : FOR_DIRECTIONS_INLINE(dir_init) {}
#undef dir_init
};
*/

template <Angles CWAngle, typename TImageContainer>
class PixelNeighborhoodDomainAdapter {
  typedef DGtal::Z2i::KSpace KSpace;
  typedef TImageContainer ImageContainer;
  typedef typename TImageContainer::Domain Domain;
  typedef typename TImageContainer::Point Point;
  typedef typename TImageContainer::Value Value;  
  typedef Directions<CWAngle> dirs;

  PixelNeighborhoodDomainAdapter(const Point aCenterPoint, 
    DGtal::ConstAlias<ImageContainer> anImage,
    DGtal::ConstAlias<KSpace> aKSpace) { }

 private:
  const Point centerPoint;
  const ImageContainer* myImagePtr;
};

template <typename TImageContainer, typename TFunctorD>
class PixelNeighborhoodImageAdapter : public DGtal::ConstImageAdapter2<TImageContainer, typename TImageContainer::Domain, DGtal::functors::Identity, typename TImageContainer::Value, DGtal::functors::Identity> {
  typedef PixelNeighborhoodImageAdapter<TImageContainer, TFunctorD> Self;
  typedef TImageContainer ImageContainer;
  typedef typename TImageContainer::Domain Domain;
  typedef typename TImageContainer::Point Point;
  typedef typename TImageContainer::Value Value;
  typedef DGtal::Z2i::KSpace KSpace;
  typedef DGtal::functors::Identity Identity;
  typedef DGtal::ConstImageAdapter2<ImageContainer, Domain, TFunctorD, Value, Identity> Base;
  typedef typename Base::ConstRange ConstRange;
  
  static constexpr Identity id();

      PixelNeighborhoodImageAdapter(DGtal::ConstAlias<ImageContainer> anImage, 
        DGtal::ConstAlias<KSpace> aKSpace,
                      DGtal::ConstAlias<Domain> aDomain,
                      DGtal::ConstAlias<TFunctorD> aFD,
                      DGtal::ConstAlias<TFunctorV> aFV
      ) :
            myImagePtr(&anImage), mySubDomainPtr(&aDomain), myFD(&aFD), myFV(&aFV)
    {
      static_assert(std::is_default_constructible_v<Value>);
      if constexpr (std::is_arithmetic_v<Value>) {
        defaultValue = 0;
      }
      else {
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
    ConstImageAdapter2( const ConstImageAdapter2 & other ) = default;

    /**
    * Assignment.
    * @param other the object to copy.
    * @return a reference on 'this'.
    */
    ConstImageAdapter2 & operator= ( const ConstImageAdapter2 & other )
    {
#ifdef DEBUG_VERBOSE
        trace.warning() << "ConstImageAdapter2 assignment " << std::endl;
#endif
        if (&other != this)
        {
            myImagePtr = other.myImagePtr;
            mySubDomainPtr = other.mySubDomainPtr;
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

};



