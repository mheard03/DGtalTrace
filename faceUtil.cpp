#pragma once
#include <DGtal/base/Common.h>
#include <algorithm>
#include <array>
#include <cstdint>
#include <deque>
#include <functional>
#include <limits>
#include <type_traits>
#include <unistd.h>
#include <vector>
#include <DGtal/helpers/StdDefs.h>
#include <DGtal/base/Trace.h>
#include "util/traceHelper.cpp"

enum DimNames { POINTEL = 0, SURFEL = 1, SPEL = 2, PIXEL = 2 };

template <typename KSpace>
std::vector<typename KSpace::Cell> uAllFaces(const KSpace& K, const typename KSpace::Cell& startCell, int toDim) {
  DGtal::Dimension fromDim = K.uDim(startCell);
  if (fromDim == toDim) return { startCell };

  std::vector<typename KSpace::Cell> result;
  auto faces = (fromDim > toDim) ? K.uFaces(startCell) : K.uCoFaces(startCell);
  for (auto face : faces) {
    if (K.uDim(face) == toDim) result.push_back(face);
  }
  return result;
}


template <typename KSpace, typename Cell>
std::vector<Cell> uPixelFaces(const KSpace& K, const Cell& startCell) {
  return uAllFaces(K, startCell, 2);
}

template <typename KSpace, typename Cell>
std::vector<Cell> uSurfelFaces(const KSpace& K, const Cell& startCell) {
  return uAllFaces(K, startCell, 1);
}

template <typename KSpace, typename Cell>
std::vector<Cell> uPointelFaces(const KSpace& K, const Cell& startCell) {
  return uAllFaces(K, startCell, 0);
}

template<uint fromDim, uint toDim> constexpr uint maxFaceCount = (fromDim == toDim) ? 1 : 4;
template<> constexpr uint maxFaceCount<DimNames::SURFEL, DimNames::POINTEL> = 2;
template<> constexpr uint maxFaceCount<DimNames::SURFEL, DimNames::SPEL> = 2;

using namespace DGtal::Z2i;
void writeCells(std::vector<std::string>& names, std::vector<PreCell>& cells) {
  
  for (int i = 0; i < names.size(); i++) {
    DGtal::trace.info() << names[i] << "Cell: "
      << "dimension: " << KPreSpace::uDim(cells[i]) << ", "
      << "K-coords: " << KPreSpace::uKCoords(cells[i])
      << std::endl;
  }
}

/*
template <typename T> struct larger_int {};
template <> struct larger_int<uint8_t>  { typedef int16_t type; };
template <> struct larger_int<uint16_t> { typedef int type; };
template <> struct larger_int<uint32_t> { typedef long type; };
template <> struct larger_int<uint64_t> { typedef long long type; };
*/
namespace {
  template <typename TCell>
  inline int getDistanceToMin(const TCell& cell, const typename TCell::Point& lowerKCoords, DGtal::Dimension k) {
    return (TCell::PreCellularGridSpace::uKCoord(cell, k) - lowerKCoords[k]) >> 1;
  }
  template <typename TCell>
  inline int getDistanceToMax(const TCell& cell, const typename TCell::Point& upperKCoords, DGtal::Dimension k) {
    return (upperKCoords[k] - TCell::PreCellularGridSpace::uKCoord(cell, k)) >> 1;
  }
}

template <typename TKSpace>
std::array<std::array<typename TKSpace::Integer, 2>, TKSpace::dimension> getDistanceToEdges(const TKSpace& K, const typename TKSpace::Cell& startCell, DGtal::Dimension toDim) {
  typedef typename TKSpace::Integer TInteger;
  const Point lowerKCoords = K.uKCoords(K.lowerCell());
  const Point upperKCoords = K.uKCoords(K.upperCell());

  std::array<std::array<TInteger, 2>, TKSpace::dimension> distances;
  for (auto k = 0; k < TKSpace::dimension; k++) {
    std::fill(distances[k].begin(), distances[k].end(), std::numeric_limits<TInteger>::max());
  }

  auto cells = uAllFaces(KPreSpace(), startCell.preCell(), toDim);
  for (auto cell : cells) {
    for (auto k = 0; k < TKSpace::dimension; k++) {
      if (K.isSpacePeriodic(k)) continue;
      distances[k][0] = std::min(distances[k][0], getDistanceToMin(cell, lowerKCoords, k));
      distances[k][1] = std::min(distances[k][1], getDistanceToMax(cell, upperKCoords, k));
    }
  }

  return distances;
}

std::vector<Cell> getTopologicallyUnique(const KSpace& K, const std::vector<Cell>& cells) {
  std::vector<int> unique_topologies;
  std::vector<Cell> results;
  for (auto cell : cells) {
    auto topology = K.uTopology(cell);
    if (std::find(unique_topologies.begin(), unique_topologies.end(), topology) == unique_topologies.end()) {
      unique_topologies.push_back(topology);
      results.push_back(cell);
    }
  }
  return results;
}

struct KBounds {
  static constexpr DGtal::Dimension dimension = KSpace::dimension;
  static constexpr DGtal::Dimension DIM = dimension;

  typedef KSpace::PreCellularGridSpace PK;
  typedef KSpace::Cell Cell;
  typedef KSpace::SCell SCell;
  typedef KSpace::PreCell PreCell;
  typedef KSpace::SPreCell SPreCell;
  typedef std::array<uint, KSpace::dimension> MarginsArray;

 private:
  template <typename T> struct isCell : std::false_type {};
  template <> struct isCell<Cell> : std::true_type {};
  template <> struct isCell<PreCell> : std::true_type {};
  template <> struct isCell<SCell> : std::true_type {};
  template <> struct isCell<SPreCell> : std::true_type {};  

  PreCell pc(const PreCell& c) { return c; }
  PreCell pc(const SPreCell& c) { return PK::unsigns(c); }
  PreCell pc(const Cell& c) { return pc(c.preCell()); }
  PreCell pc(const SCell& c) { return pc(c.preCell()); }

  template <typename T> struct isUint : std::conjunction<std::is_unsigned<T>, std::is_integral<T>> {};
  template <typename T> struct isUintArray : std::false_type {};
  template <typename T> struct isUintArray<std::array<T, KSpace::dimension>> : isUint<T> {}; 
  template <typename T> struct isMargin : std::disjunction<isUint<T>, isUintArray<T>> {};

  MarginsArray ma(const MarginsArray& m) { return m; }
  
  template <typename T, std::enable_if_t<isUint<T>::value, bool> = true>
  MarginsArray ma(const T scalar) { 
    MarginsArray a;
    std::fill(a.begin(), a.end(), scalar);
    return a; 
  }

  template <typename T, std::enable_if_t<isUintArray<T>::value, bool> = true>
  MarginsArray ma(const T& array) { 
    MarginsArray a;
    std::copy(a.begin(), a.end(), array);
    return a; 
  }


  const KSpace* myKSpace;
  std::array<uint, KSpace::dimension> myMargins;
  PreCell myLowerCell;
  PreCell myUpperCell;


 public:
  template <typename TCell, typename TMargin, typename = std::enable_if_t<std::disjunction_v<isCell<TCell>, isMargin<TMargin>>>>
  KBounds(const TCell& lowerCell, 
          const TCell& upperCell, 
          const KSpace& K, 
          TMargin margins) : 
      myLowerCell(pc(lowerCell)), myUpperCell(pc(upperCell)), myKSpace(&K), myMargins(ma(margins)) {}
  
  template <typename TCell, typename TMargin, typename = std::enable_if_t<std::disjunction_v<isCell<TCell>, isMargin<TMargin>>>>
  KBounds(const TCell& p, 
          const KSpace& K, 
          TMargin margins) : KBounds(K.uFirst(p).preCell(), K.uLast(p).preCell(), K, margins) {}
    
  bool isValid() {
    if (!myKSpace->uIsInside(myLowerCell)) return false;
    if (!myKSpace->uIsInside(myUpperCell)) return false;
    
    for (int k = 0; k < dimension; k++) {
      if (myKSpace->isSpacePeriodic(k)) continue;
      if (myLowerCell.coordinates[k] > myUpperCell.coordinates[k]) return false;
    }
    return true;
  }


};

std::vector<Cell> getTopologicallyUnique(const KSpace& K) {
/*
  auto writeCell = [](const Cell& cell) -> void {
    std::vector<int> dirs;
    for ( auto q = PK::uDirs(cell); q != 0; ++q ) {
      dirs.push_back(*q);
    }

    DGtal::trace.info() << cell
      << ": dimension: " << KPreSpace::uDim(cell)
      << ", dirs: " << dirs
      << ", topology: " << KPreSpace::uTopology(cell) << std::endl;
  };
*/
  typedef KSpace::PreCellularGridSpace PK;
  auto topoCompare = [](const Cell& a, const Cell& b) -> const bool { return PK::uTopology(a) < PK::uTopology(b); };
  std::set<Cell, decltype(topoCompare)> topologicallyUnique(topoCompare);

  for (auto cell : { K.uSpel(K.lowerBound()), K.uSpel(K.upperBound()) }) {
    auto faces =  K.uFaces(cell);    
    topologicallyUnique.insert(cell);
    topologicallyUnique.insert(faces.cbegin(), faces.cend());
  }

  std::vector<Cell> result(topologicallyUnique.cbegin(), topologicallyUnique.cend());
  return result;
}

std::pair<PreCell, PreCell> getCellRange(const PreCell& lowerCell, const PreCell& upperCell, const KSpace& K, const std::array<uint, KSpace::dimension> margins) {
  typedef KSpace::PreCellularGridSpace PK;
  auto newLowerCell(lowerCell);
  auto newUpperCell(upperCell);

  for (int k = 0; k < KSpace::dimension; k++) {
    if (!K.isSpacePeriodic(k)) newLowerCell = PK::uGetAdd(newLowerCell, k, margins[k]);
    if (!K.isSpacePeriodic(k)) newUpperCell = PK::uGetSub(newUpperCell, k, margins[k]);
  }

  return std::pair<PreCell, PreCell>(newLowerCell, newUpperCell);
}

std::pair<PreCell, PreCell> getCellRange(const PreCell& lowerCell, const PreCell& upperCell, const KSpace& K, const uint margin) {
  std::array<uint, KSpace::dimension> margins;
  std::fill(margins.begin(), margins.end(), margin);
  return getCellRange(lowerCell, upperCell, K, margins);
}

std::pair<PreCell, PreCell> getCellRange(const PreCell& p, const KSpace& K, const std::array<uint, KSpace::dimension> margins) {
  typedef KSpace::PreCellularGridSpace PK;
  auto lowerCell = K.uFirst(p).preCell();
  auto upperCell = K.uLast(p).preCell();
  return getCellRange(lowerCell, upperCell, K, margins);
}

std::pair<PreCell, PreCell> getCellRange(const PreCell& p, const KSpace& K, const uint margin = 0) {
  std::array<uint, KSpace::dimension> margins;
  std::fill(margins.begin(), margins.end(), margin);
  return getCellRange(p, K, margins);
}