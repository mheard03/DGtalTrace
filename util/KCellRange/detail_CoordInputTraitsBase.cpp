#pragma once
#include <concepts>
#include <DGtal/base/Common.h>
#include <DGtal/kernel/PointVector.h>
#include <initializer_list>
#include <iterator>
#include <type_traits>
#include "../DimensionOf.cpp"
#include "../concepts/CCell.cpp"
#include "../concepts/CIsInitializerList.cpp"
#include "../concepts/CIsFixedSize.cpp"

namespace DGtalTrace {

namespace {
template <typename T>
concept CIsSubscriptable = requires(T& t, DGtal::Dimension k) {
  { t[k] };
  { t[k] = std::declval<std::remove_cv_t<decltype(std::declval<T>()[0])>>() };
  DimensionOf<T>;
};


/*
template <typename T>
concept CIsInitializerList = requires {
  typename T::value_type;
  requires (std::integral<typename T::value_type> || std::floating_point<typename T::value_type>);
  requires std::same_as<T, std::initializer_list<typename T::value_type>>;
  requires requires { T::size() > 0; };
};
*/
} // anonymous namespace

template <typename T>
struct CoordInputTraits;

namespace detail {
template <typename T>
struct CoordInputTraitsBase;
}

namespace concepts {
namespace detail {

template <typename T>
concept CCoordInputTraitsBase = requires {
  typename T::AdaptedType;
  typename T::Component;
  typename T::Point;
  requires CPoint<typename T::Point>;
} && requires (typename T::AdaptedType adapted, DGtal::Dimension k) {
  { T::get(adapted, k) } -> detail::IsBasically<typename T::Component>;
//  { T::get(adapted) } -> detail::IsBasically<typename T::Point>;
};

} // namespace detail

template <typename InputType>
concept CHasCoordInputTraits = requires {
  requires !std::same_as<detail::CoordInputTraitsBase<InputType>, void>;
  // requires detail::CCoordInputTraitsBase<CoordInputTraits<InputType>>;
};

template <class T, typename... TArgs>
concept CIsNonTriviallyConstructibleFrom = std::constructible_from<T, TArgs...> 
  && (sizeof...(TArgs) > 0)
  && (!concepts::detail::IsBasically<T, std::tuple_element_t<0, std::tuple<TArgs...>>>);

} // namespace concepts

namespace detail {

template <CIsSubscriptable T>
struct CoordInputTraitsBase<T> {
  using AdaptedType = T;
  using Component = std::remove_cvref_t<decltype(std::declval<T>()[0])>;
  using Point = DGtal::PointVector<DimensionOf<T>, Component>;

  static inline const Component get(const AdaptedType& t, const DGtal::Dimension k) { 
    return t[k];
  }

  template <typename... TArgs>
  requires concepts::CIsNonTriviallyConstructibleFrom<AdaptedType, TArgs...>  
  static inline const Component get(const TArgs&... args, const DGtal::Dimension k) { 
    return t[k];
  }
};


}

template <CIsSubscriptable T>
struct CoordInputTraits<T> {
  using AdaptedType = T;
  using Component = std::remove_cvref_t<decltype(std::declval<T>()[0])>;
  using Point = DGtal::PointVector<DimensionOf<T>, Component>;

  static inline const Component get(const AdaptedType& t, const DGtal::Dimension k) { 
    return t[k];
  }

  template <typename... TArgs>
  requires concepts::CIsNonTriviallyConstructibleFrom<AdaptedType, TArgs...>  
  static inline const Component get(const TArgs&... args, const DGtal::Dimension k) { 
    return t[k];
  }
    
  GET_ALL
};


#define GET_ALL \
static const Point get(const AdaptedType& obj) { \
  Point result; \
  for (const DGtal::Dimension k : dimsOf(obj)) { \
    result[k] = get(obj, k); \
  } \
  return result; \
} \
static inline const Component get(const Point& t, const DGtal::Dimension k) requires (!std::same_as<AdaptedType, Point>) { \
  return t[k]; \
} \
static const Point get(const Point& p) requires (!std::same_as<AdaptedType, Point>) { \
  return p; \
} \

//auto a = {1, 2, 3};
//auto b = std::begin(a)[2];

//std::array<int, 3> b = a;


template <typename AdaptedType, typename Component,  DGtal::Dimension dim = DimensionOf<AdaptedType>>
struct CoordInputTraitsCommon {
  static inline const Component get(const AdaptedType& t, const DGtal::Dimension k) { 
    return t[k];
  }

  template <typename... TArgs>
  requires concepts::CIsNonTriviallyConstructibleFrom<AdaptedType, TArgs...>  
  static inline const Component get(const TArgs&... args, const DGtal::Dimension k) { 
    AdaptedType 
    return t[k];
  }
};


template <CIsSubscriptable T>
struct CoordInputTraits<T> {
  using AdaptedType = T;
  using Component = std::remove_cvref_t<decltype(std::declval<T>()[0])>;
  using Point = DGtal::PointVector<DimensionOf<T>, Component>;

  static inline const Component get(const AdaptedType& t, const DGtal::Dimension k) { 
    return t[k];
  }

  template <typename... TArgs>
  requires concepts::CIsNonTriviallyConstructibleFrom<AdaptedType, TArgs...>  
  static inline const Component get(const TArgs&... args, const DGtal::Dimension k) { 
    return t[k];
  }
    
  GET_ALL
};

/*
template <CIsInitializerList T>
struct CoordInputTraits<T> {
  using AdaptedType = T;
  using Component = std::remove_cvref_t<typename T::value_type>;
  using Point = DGtal::PointVector<T::size(), Component>;

  static inline const Component get(const AdaptedType& t, const DGtal::Dimension k) { 
    return std::begin(t)[k];
  }
  GET_ALL
};
*/

template <concepts::CXXCell TCell>
struct CoordInputTraits<TCell> {
  using AdaptedType = TCell;
  using Component = typename TCell::Point::Component;
  using Point = typename TCell::Point;

  using KPreSpace = typename TCell::PreCellularGridSpace;

  static inline const Component get(const TCell& c, const DGtal::Dimension k) { 
    if constexpr (concepts::CUXCell<TCell>) {
      return KPreSpace::uKCoord(c, k);
    }
    if constexpr (concepts::CSXCell<TCell>) {
      return KPreSpace::sKCoord(c, k);
    }
  }
  GET_ALL
};

#undef GET_ALL
} // namespace DGtalTrace