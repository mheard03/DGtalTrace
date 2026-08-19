#pragma once
#include <DGtal/base/Assert.h>
#include <DGtal/kernel/NumberTraits.h>
#include <DGtal/topology/KhalimskySpaceND.h>
#include <concepts>
#include <optional>
#include <string>
#include <type_traits>
#include <utility>
#include <boost/concept/assert.hpp>
#include <DGtal/base/Common.h>
#include <DGtal/base/ConstAlias.h>
#include <DGtal/helpers/StdDefs.h>  // <-- remove me
#include "../concepts/BaseConcepts.cpp"
#include "../concepts/CCell.cpp"
#include "../concepts/CXCellularGridSpaceND.cpp"
#include "Globals.cpp"
#include "CellTypeTraits.cpp"
#include "detail/OrthogonalDirections.cpp"
#include "KCoordsAdjuster.cpp"
#include "KPreCellNavigator.cpp"

namespace DGtalTrace {

using TKSpace = DGtal::Z2i::KSpace;
// template <concepts::CXCellularGridSpaceND TKSpace>
// requires (DimensionOf<KPreSpace> == 2)
class KCellNavigator {
 public:
  typedef TKSpace KSpace;
  typedef KSpace::PreCellularGridSpace KPreSpace;
  typedef KCoordsAdjuster<KSpace> KCA;
  typedef KSpace::Point Point;
  typedef KSpace::Vector Vector;
  typedef KSpace::Integer Integer;
  typedef KSpace::Cell  Cell;
  typedef KSpace::SCell SCell;
  typedef std::optional<Cell> OptCell;
  typedef std::optional<SCell> OptSCell;

private:
  enum class CellStates { INVALID, IMPROPER, VALID };
  typedef KPreCellNavigator KPCN;

  template <concepts::CXXCell TCell>
  CellStates getCellState(const TCell& cell) {
    if (!myKPtr->isAnyDimensionPeriodic()) {
      // If there's no periodicity at all, simply test and return valid/invalid
      return myKCA.isValid(cell) ? CellStates::VALID : CellStates::INVALID;
    }
    else {
      bool isInside = true;
      for (DGtal::Dimension k = 0; k < KSpace::dimension; ++k) {
        if (!myKCA.isInside(cell)) {
          if (myKPtr->isSpacePeriodic(k)) {
            // Cell is out of bounds on a periodic dimension: record the fact
            isInside = false;
          }
          else {
            // Cell is out of bounds on a non-periodic dimension: early eject
            return CellStates::INVALID;
          }
        }
      }
      return isInside ? CellStates::VALID : CellStates::IMPROPER;
    }    
  }

/*
  template <concepts::CXXCell TCell>
  inline auto upgradeOrNull(const TCell& cell) {
    constexpr bool isSigned = concepts::CSXCell<TCell>;
    typedef std::conditional_t<isSigned, OptSCell, OptCell> ResultType;
    ResultType result;

    if (myKCA.isInside(cell)) {
      if constexpr (isSigned) 
        result = myKPtr->sCell(cell);
      else
        result = myKPtr->uCell(cell);
    }
    return result;
  }
*/
inline std::optional<Cell> upgradeOrNull(const KPreSpace::Cell& cell) {
  if (getCellState(cell) == CellStates::INVALID) return std::nullopt;
  return std::optional<Cell>(myKPtr->uCell(cell));
}
inline std::optional<SCell> upgradeOrNull(const KPreSpace::SCell& cell) {
  if (getCellState(cell) == CellStates::INVALID) return std::nullopt;
  return std::optional<SCell>(myKPtr->sCell(cell));
}

public:
  KCellNavigator(DGtal::ConstAlias<KSpace> ks) : myKPtr(&ks), myKCA(&ks) {}

  // ----- Surfel: Get incident pointels  -------------------------------------
  inline OptCell surfelHead(const SCell& surfel) {
    return upgradeOrNull(KPCN::surfelHead(surfel));
  }

  template <concepts::CSXCell TSCell>
  inline OptSCell surfelTail(const TSCell& surfel) {
    return upgradeOrNull(KPCN::surfelHead(surfel));
  }

  template <concepts::CXXCell TCell>
  inline std::array<Cell, 2> surfelIncidentPointels(const TCell& surfel) {
    return upgradeOrNull(KPCN::surfelIncidentPointels(surfel));
  }

  template <concepts::CSXCell TSCell>
  inline static SCell surfelInteriorSpel(const TSCell& surfel) {
    assertSurfel(surfel);
    auto d = KSpace::sOrthDir(surfel);
    return KSpace::sIncident(surfel, d, KSpace::sDirect(surfel, d));
  }

  template <concepts::CSXCell TSCell>
  inline static SCell surfelExteriorSpel(const TSCell& surfel) {
    assertSurfel(surfel);
    auto d = KSpace::sOrthDir(surfel);
    return KSpace::sIncident(surfel, d, KSpace::sDirect(surfel, -d));
  }

private:
  const KSpace* myKPtr;
  const KCoordsAdjuster<KSpace> myKCA;
};

} // namespace DGtalTrace

*/

} // endnamespace DGtalTrace