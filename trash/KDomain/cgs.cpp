#pragma once
#include <DGtal/helpers/StdDefs.h>
#include <concepts>
#include <tuple>
#include "../CellTypeTraits.cpp"
#include "../concepts/CCell.cpp"
#include "../concepts/CXCellularGridSpaceND.cpp"
#include "setKCoords_unsafe.cpp"

namespace DGtalTrace {

namespace cgs {
namespace {
  template <typename T>
  requires concepts::CXXCell<T> || concepts::CXCellularGridSpaceND<T>
  using KSpace = T::CellularGridSpace;

  template <typename T>
  requires concepts::CXXCell<T> || concepts::CXCellularGridSpaceND<T>
  using KPreSpace = T::PreCellularGridSpace;

  template <typename T>
  requires concepts::CXXCell<T> || concepts::CXCellularGridSpaceND<T>
  KPreSpace<T> kps = detail::ksInfinite<KPreSpace<T>>;

  template <typename T>
  requires concepts::CXXCell<T> || concepts::CXCellularGridSpaceND<T>
  using Sign = KSpace<T>::Sign;

  template <typename T>
  requires concepts::CXXCell<T> || concepts::CXCellularGridSpaceND<T>
  using Point = KSpace<T>::Point;

  template <typename T>
  requires concepts::CXXCell<T> || concepts::CXCellularGridSpaceND<T>
  using Vector = KSpace<T>::Vector;

  template <typename T>
  requires concepts::CXXCell<T> || concepts::CXCellularGridSpaceND<T>
  using Integer = KSpace<T>::Integer;


  template <typename Function, typename ReturnType, typename... ParamTypes>
  concept CCanCall = requires(Function fn, ParamTypes... args) {
      { fn(args...) } -> std::convertible_to<ReturnType>; // std::same_as<ReturnType>;
  };

  // gg Claude
  template <typename Tuple, typename ReturnType, typename... ParamTypes>
  consteval std::size_t first_callable_index() {
    using T = std::remove_cvref_t<Tuple>;
    constexpr std::size_t N = std::tuple_size_v<T>;

    constexpr auto matches = []<std::size_t... Is>(std::index_sequence<Is...>) {
        return std::array{ CCanCall<std::tuple_element_t<Is, T>, ReturnType, ParamTypes...>... };
    }(std::make_index_sequence<N>{});

    for (std::size_t i = 0; i < N; ++i) {
      if (matches[i]) return i;
    }

    return N; // not found
  };

  DGtal::Z2i::KSpace ks;
  auto t = std::make_tuple(ks.uCoord, ks.uCoords, ks.sCoord, ks.sCoords);

}



// ----------------------- kCoords (plural)  ----------------------------------
template <concepts::CXXCell TCell>
inline Point<TCell> kCoords(const TCell& cell) {
  if constexpr (concepts::CUXCell<TCell>) {
    return KPreSpace<TCell>::uKCoords(cell);
  }
  if constexpr (concepts::CSXCell<TCell>) {
    return KPreSpace<TCell>::KPreSpace::sKCoords(cell);
  }
}
template <concepts::CXXCell TCell, concepts::CCellularGridSpaceND TKSpace>
inline void setKCoords(TCell& cell, const Point<TCell>& kCoords, const TKSpace& ks) {
  if constexpr (concepts::CUCell<TCell>) {
    ks.uSetKCoords(cell, kCoords);
  }
  if constexpr (concepts::CSCell<TCell>) {
    ks.sSetKCoords(cell, kCoords);
  }
}
template <concepts::CXXCell TCell, concepts::CPreCellularGridSpaceND TKSpace>
inline void setKCoords(TCell& cell, const Point<TCell>& kCoords) {
  setKCoords(cell, kCoords, kps<TCell>);
}


// ----------------------- kCoord (singular)  ---------------------------------
template <concepts::CXXCell TCell>
inline TCell::Integer kCoord(const TCell& cell, const DGtal::Dimension k) {
  if constexpr (concepts::CUXCell<TCell>) {
    return KPreSpace<TCell>::uKCoord(cell, k);
  }
  if constexpr (concepts::CSXCell<TCell>) {
    return KPreSpace<TCell>::sKCoord(cell, k);
  }
}
template <concepts::CXXCell TCell, concepts::CCellularGridSpaceND TKSpace>
inline void setKCoord(TCell& cell, const DGtal::Dimension k, const Integer<TCell> value, const TKSpace& ks) {
  if constexpr (concepts::CUXCell<TCell>) {
    ks.uSetKCoord(cell, k, value);
  }
  if constexpr (concepts::CSXCell<TCell>) {
    ks.sSetKCoord(cell, k, value);
  }
}
template <concepts::CXXCell TCell, concepts::CPreCellularGridSpaceND TKSpace>
inline void setKCoord(TCell& cell, const DGtal::Dimension k, const Integer<TCell> value) {
  setKCoord(cell, k, value, kps<TCell>);
}


// ----------------------- isValid  -------------------------------------------
template <concepts::CCellularGridSpaceND TKSpace>
inline bool isValid(const Point<TKSpace>& kCoords, const TKSpace& ks) {
  return ks.cIsValid(kCoords);
}
template <concepts::CXXCell TCell, concepts::CCellularGridSpaceND TKSpace>
inline bool isValid(const TCell& cell, const TKSpace& ks) {
  if constexpr (concepts::CUXCell<TCell>) {
    return ks.uIsValid(cell);
  }
  if constexpr (concepts::CSXCell<TCell>) {
    return ks.sIsValid(cell);
  }
  return isValid(kCoords(cell), ks);
}

template <concepts::CPreCellularGridSpaceND TKSpace>
inline constexpr bool isValid(const Point<TKSpace>& kCoords, const TKSpace& ks) {
  return true;
}
template <concepts::CXXCell TCell, concepts::CPreCellularGridSpaceND TKSpace>
inline constexpr bool isValid(const TCell& cell, const TKSpace& ks) {
  return true;
}


// ----------------------- isInside  ------------------------------------------
template <concepts::CCellularGridSpaceND TKSpace>
inline bool isInside(const Point<TKSpace>& kCoords, const TKSpace& ks) {
  return ks.cIsInside(kCoords);
}
template <concepts::CXCell TCell, concepts::CCellularGridSpaceND TKSpace>
inline bool isInside(const TCell& cell, const TKSpace& ks) {
  if constexpr (concepts::CUCell<TCell>) {
    return ks.uIsInside(cell);
  }
  if constexpr (concepts::CSCell<TCell>) {
    return ks.sIsInside(cell);
  }
  return isInside(kCoords(cell), ks);
}

template <concepts::CPreCellularGridSpaceND TKSpace>
inline constexpr bool isInside(const Point<TKSpace>& kCoords, const TKSpace& ks) {
  return true;
}
template <concepts::CXXCell TCell, concepts::CPreCellularGridSpaceND TKSpace>
inline constexpr bool isInside(const TCell& cell, const TKSpace& ks) {
  return true;
}


// ----------------------- sign  --------------------------------------------
template <concepts::CSXCell TCell>
inline Sign<TCell> sign(const TCell& cell) {
  return KPreSpace<TCell>::sSign(cell);
}

template <concepts::CXXCell TCell>
inline void setSign(TCell& cell, const Sign<TCell> sign) {
  KPreSpace<TCell>::sSetSign(cell, sign);
}


// ----------------------- cell  --------------------------------------------
template <concepts::CSXCell TCell>
inline TCell cell(const Point<TCell>& kCoords, const Sign<TCell>& sign, const KSpace<TCell>& ks) {
  return ks.sCell(kCoords, sign);
}
template <concepts::CSXCell TCell>
inline TCell cell(const Point<TCell>& kCoords, const KSpace<TCell>& ks) {
  return cell<TCell>(kCoords, Sign<TCell>::POS, ks);
}
template <concepts::CSPreCell TCell>
inline TCell cell(const Point<TCell>& kCoords, const Sign<TCell>& sign) {
  return cell<TCell>(kCoords, sign, kps<TCell>);
}
template <concepts::CSPreCell TCell>
inline TCell cell(const Point<TCell>& kCoords) {
  return cell<TCell>(kCoords, Sign<TCell>::POS);
}

template <concepts::CUXCell TCell, concepts::CCellularGridSpaceND TKSpace>
inline TCell cell(const Point<TCell>& kCoords, const TKSpace& ks) {
  return ks.uCell(kCoords);
}
template <concepts::CUPreCell TCell>
inline TCell cell(const Point<TCell>& kCoords) {
  return cell<TCell>(kCoords, kps<TCell>);
}


requires concepts::CHasKPreSpace<KPreSpace, TCell>
static TCell make(const Point& kCoords) {
  return make_inner<TCell>(kCoords, preKPtr);
}
template <concepts::CSPreCell TCell>
requires concepts::CHasKPreSpace<KPreSpace, TCell>
static TCell make(const Point& kCoords, const Sign sign) {
  return make_inner<TCell>(kCoords, sign, preKPtr);
}

template <concepts::CSXCell TCell>
inline Sign<TCell> sign(const TCell& cell) {
  return KPreSpace<TCell>::sSign(cell);
}

template <concepts::CXXCell TCell>
inline void setSign(TCell& cell, const Sign<TCell> sign) {
  KPreSpace<TCell>::sSetSign(cell, sign);
}





#undef TYPEDEFS
#undef CK_TYPEDEFS
#undef K_TYPEDEFS
#undef C_TYPEDEFS
} // namespace cgs
} // namespace DGtalTrace

