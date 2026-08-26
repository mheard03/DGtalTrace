#pragma once
#include <DGtal/base/Assert.h>
#include <DGtal/kernel/NumberTraits.h>
#include <DGtal/topology/KhalimskySpaceND.h>
#include <algorithm>
#include <concepts>
#include <optional>
#include <string>
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
#include "detail/CellDeltaHelper.cpp"
#include "KCoordsAdjuster.cpp"

namespace DGtalTrace {

// using TKPreSpace = DGtal::Z2i::KPreSpace;
template <concepts::CPreCellularGridSpaceND TKPreSpace>
requires (DimensionOf<TKPreSpace> == 2)
class KCellNavigator;

template <concepts::CPreCellularGridSpaceND TKPreSpace>
requires (DimensionOf<TKPreSpace> == 2)
class KPreCellNavigator {
 public:  
  typedef TKPreSpace KPreSpace;
  typedef KCoordsAdjuster<KPreSpace> KCA;
  typedef CellDelta<KPreSpace> Delta;
  typedef detail::CellDeltaHelper<KPreSpace> CDH;
  typedef KPreSpace::Point Point;
  typedef KPreSpace::Vector Vector;
  typedef KPreSpace::Sign Sign;
  typedef KPreSpace::Integer Integer;
  typedef KPreSpace::Cell  Cell;
  typedef KPreSpace::SCell SCell;
  
private:
  template <concepts::CXXCell TCell>
  using PreSelf = CellTypeTraits<TCell>::PreSelf;
  
  template <concepts::CXXCell TCell>
  inline static void validateCell(const TCell& cell, const DGtal::Dimension dim, const std::optional<bool> hasSign = std::nullopt) {
    static constexpr std::array<std::string, 3> types = { "pointel", "surfel", "spel" };
    ASSERT_MSG(KCA::dim(cell) == dim, std::string() + "Expected " + types[dim] + " argument, got dim " + std::to_string(KCA::dim(cell)));
    if (hasSign.has_value()) {
      if (hasSign.value()) 
        ASSERT_MSG(concepts::CSXCell<TCell>, "Expected signed " + types[dim]);
      else
        ASSERT_MSG(concepts::CUXCell<TCell>, "Expected unsigned " + types[dim]);
    }
  }

  template <concepts::CXXCell TCell>
  inline static auto dirs(const TCell& surfel) {
    if constexpr (concepts::CSXCell<TCell>) {
      return *KPreSpace::sDirs(surfel);
    }
    else {
      return *KPreSpace::uDirs(surfel);
    }
  }

  template <concepts::CXXCell TCell>
  inline static auto orthDirs(const TCell& surfel) {
    if constexpr (concepts::CSXCell<TCell>) {
      return *KPreSpace::sOrthDirs(surfel);
    }
    else {
      return *KPreSpace::uOrthDirs(surfel);
    }
  }

  template <concepts::CXXCell TCell>
  inline static std::vector<PreSelf<TCell>> incident(const TCell& cell, DGtal::Dimension k, bool up) {
    if constexpr (concepts::CSXCell<TCell>) {
      return KPreSpace::sIncident(cell, k, up);
    }
    else {
      return KPreSpace::uIncident(cell, k, up);
    }
  }

  static std::vector<Cell> unsign(const std::vector<SCell>& cells) {
    std::vector<Cell> result;
    result.reserve(cells.size());
    std::transform(cells.begin(), cells.end, result.begin(), [](SCell& cell) { return KCA::unsigns(cell); });
    return result;
  }

public:
  // ----- Surfel: Get incident pointels  -------------------------------------
  inline static Cell surfelHead(const SCell& surfel) {
    validateCell(surfel, 1, true);
    return KCA::unsigns(KPreSpace::sDirectIncident(surfel, *KPreSpace::sDirs(surfel)));
  }

  inline static Cell surfelTail(const SCell& surfel) {
    validateCell(surfel, 1, true);
    return KCA::unsigns(KPreSpace::sIndirectIncident(surfel, *KPreSpace::sDirs(surfel)));
  }

  template <concepts::CUXCell TCell>
  static std::vector<Cell> surfelIncidentPointels(const TCell& surfel) {
    validateCell(surfel, 1);
    DGtal::Dimension k = *dirs(surfel);
    return {
      incident(surfel, k, false),
      incident(surfel, k, true)
    };
  }

  template <concepts::CXXCell TCell>
  static SCell makeSurfel(const TCell& tailPointel, const TCell& headPointel) {
    validateCell(tailPointel, 0);
    validateCell(headPointel, 0);

    auto delta = CDH::getDelta(tailPointel, headPointel);
    return KCA::sShifted(tailPointel, delta.toMag(1));
  }

  // ----- Surfel: Set head/tail pointel  -------------------------------------
  template <concepts::CXXCell TSurfel, concepts::CXXCell TPointel>
  static SCell setSurfelHead(const TSurfel& surfel, const TPointel& headPointel) {
    validateCell(surfel, 1);
    validateCell(headPointel, 0);

    auto delta = CDH::getDelta(surfel, headPointel);
    SCell result = KCA::sCell(KCA::kCoords(surfel), delta.sign);
    ASSERT_MSG(surfelHead(result) == headPointel, "I am not so smart");
    return result;
  }

  template <concepts::CXXCell TSurfel, concepts::CXXCell TPointel>
  static SCell setSurfelTail(const TSurfel& surfel, const TPointel& tailPointel) {
    validateCell(surfel, 1);
    validateCell(tailPointel, 0);

    auto delta = CDH::getDelta(tailPointel, surfel);
    SCell result = KCA::sCell(KCA::kCoords(surfel), delta.sign);
    ASSERT_MSG(surfelTail(result) == tailPointel, "I am not so smart");
    return result;
  }

  // ----- Surfel: Get incident spels  ----------------------------------------
  static Cell surfelInteriorSpel(const SCell& surfel) {
    validateCell(surfel, 1, true);
    return KCA::uShifted(surfel, CDH::getInwardDelta(surfel, 1));
  }

  static Cell surfelExteriorSpel(const SCell& surfel) {
    validateCell(surfel, 1, true);
    return KCA::uShifted(surfel, CDH::getInwardDelta(surfel, -1));
  }

  template <concepts::CXXCell TCell>
  static std::array<Cell, 2> surfelIncidentSpels(const TCell& surfel) {
    validateCell(surfel, 1, true);

    auto delta = CDH::getInwardDelta(KCA::sCell(surfel, true), 1);
    return {
      KCA::uShifted(surfel, delta),
      KCA::uShifted(surfel, delta.opp())
    };
  }

  // ----- Pointel: Get departing surfels  ------------------------------------
  static std::vector<SCell> pointelDepartingSurfels(const Cell& pointel) {
    validateCell(pointel, 0, true);
    std::vector<SCell> result;
    result.reserve(4);
    for (auto uSurfel : KPreSpace::uUpperIncident(pointel)) {
      result.push_back(setSurfelTail(uSurfel, pointel));
    }
    return result;
  }

  static std::vector<SCell> pointelArrivingSurfels(const Cell& pointel) {
    validateCell(pointel, 0, true);
    std::vector<SCell> result;
    result.reserve(4);
    for (auto uSurfel : KPreSpace::uUpperIncident(pointel)) {
      result.push_back(setSurfelHead(uSurfel, pointel));
    }
    return result;
  }

};


} // endnamespace DGtalTrace