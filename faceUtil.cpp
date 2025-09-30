#pragma once
#include <vector>
#include <DGtal/helpers/StdDefs.h>

using namespace DGtal;
using namespace Z2i;

namespace {
  std::vector<Cell> uAllFaces(const KSpace& K, const Cell& startCell, DGtal::Dimension toDim) {
    DGtal::Dimension fromDim = K.uDim(startCell);
    if (fromDim == toDim) return { startCell };

    std::vector<Cell> result;
    auto faces = (fromDim > toDim) ? K.uFaces(startCell) : K.uCoFaces(startCell) ;
    for (auto face : faces) {
      if (K.uDim(face) == toDim) result.push_back(face);
    }
    return result;
  }
}

std::vector<Cell> uPixelFaces(const KSpace& K, const Cell& startCell) {
  return uAllFaces(K, startCell, 2);
}

std::vector<Cell> uSurfelFaces(const KSpace& K, const Cell& startCell) {
  return uAllFaces(K, startCell, 1);
}

std::vector<Cell> uPointelFaces(const KSpace& K, const Cell& startCell) {
  return uAllFaces(K, startCell, 0);
}


/*
	if constexpr (gravity == 9.8) // now using constexpr if
		std::cout << "Gravity is normal.\n";
	else
		std::cout << "We are not on Earth.\n";
    constexpr int (*add_ptr)(int, int) = &add;
    constexpr decltype(auto) f() { return m.f(); }

*/