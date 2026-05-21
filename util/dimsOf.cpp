#pragma once
#include <concepts>
#include <numeric>
#include <ranges>
#include <DGtal/base/Common.h>
#include "concepts/CHasDimension.cpp"
#include "concepts/CXCellularGridSpaceND.cpp"
#include "DimensionOf.cpp"

namespace DGtalTrace {

namespace {
  using DimsVector = std::vector<DGtal::Dimension>;

  template <DGtal::Dimension N>
  using DimsArray = std::array<DGtal::Dimension, N>;
  
  template <concepts::CHasDimension T>
  using DimsArrayOf = DimsArray<DimensionOf<T>>;

  template <DGtal::Dimension N>
  constexpr DimsArray<N> staticDimsArray{[]() constexpr{
    DimsArray<N> result;
    std::iota(result.begin(), result.end(), 0);
    return result;
  }()};
}

template <concepts::CHasDimension T>
constexpr DimsArrayOf<T> dimsOf() {
  return staticDimsArray<DimensionOf<T>>;
}

template <concepts::CHasDimension T>
constexpr DimsArrayOf<T> dimsOf(const T& t) {
  return dimsOf<T>();
}

namespace {
  template <concepts::CXCellularGridSpaceND TKSpace>
  auto getDimsByPeriodicity(const TKSpace& ks, const bool periodicity) {
    return dimsOf<TKSpace>() | std::views::filter([&ks, periodicity](DGtal::Dimension k) { 
      if constexpr (concepts::CPreCellularGridSpaceND<TKSpace>) return true;
      return ks.isSpacePeriodic(k) == periodicity; 
    });
  }
}

template <concepts::CXCellularGridSpaceND TKSpace>
const DimsVector periodicDimsOf(const TKSpace& ks) {
  auto v = getDimsByPeriodicity(ks, true);
  return DimsVector(v.begin(), v.end()); 
}
template <concepts::CXCellularGridSpaceND TKSpace>
const DimsVector nonPeriodicDimsOf(const TKSpace& ks) {
  if constexpr (concepts::CPreCellularGridSpaceND<TKSpace>) return DimsVector();  
  auto v = getDimsByPeriodicity(ks, false);
  return DimsVector(v.begin(), v.end()); 
}
} // namespace DGtalTrace

/*
namespace DGtalTrace {

namespace {
  typedef std::ranges::iota_view<DGtal::Dimension, DGtal::Dimension> DimIotaView;
}

template <typename T>
requires requires { DimensionOf<T>; }
DimIotaView dims() {
  return DimIotaView(0, DimensionOf<T> + 1);
}

template <typename T>
requires requires { DimensionOf<T>; }
DimIotaView dims(const T& t) {
  return dims<T>();
}


namespace {
  template <concepts::CCellularGridSpaceND TKSpace>
  auto dimsByPeriodicity(const TKSpace& ks, const bool& isPeriodic) {
    return dims<TKSpace>() | std::views::filter([&](DGtal::Dimension k) { return ks.isSpacePeriodic(k) == isPeriodic; });
  }
  
  template <concepts::CCellularGridSpaceND TKSpace>
  using FilteredDimIotaView = decltype(dimsByPeriodicity(std::declval<TKSpace>, true));
}

template <concepts::CCellularGridSpaceND TKSpace>
FilteredDimIotaView<TKSpace> periodicDims(const TKSpace& ks) {
  return dims<TKSpace>() | std::views::filter([](int n) { return n % 2 == 0; });;
}



DGtal::Z2i::Point p;
auto dp = dims<DGtal::Z2i::Point>();

} // namespace DGtalTrace
*/