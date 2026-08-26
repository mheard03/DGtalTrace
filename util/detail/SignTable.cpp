#pragma once
#include "../../concepts/BaseConcepts.cpp"
#include "../../concepts/CHasDimension.cpp"
#include "../../concepts/CCell.cpp"
#include "../../concepts/CXCellularGridSpaceND.cpp"
#include "../CellTypeTraits.cpp"
#include "../Globals.cpp"
#include "../KCoordsAccessor.cpp"
#include "../traceHelper.cpp"
#include "CellDelta.cpp"
#include <DGtal/base/Assert.h>
#include <DGtal/base/Common.h>
#include <DGtal/base/ConstAlias.h>
#include <DGtal/helpers/StdDefs.h>
#include <DGtal/kernel/NumberTraits.h>
#include <utility>

namespace DGtalTrace {
namespace detail {

template <concepts::CXCellularGridSpace2D TKSpace>
struct SignTable {
  typedef TKSpace KSpace;
  typedef typename KSpace::PreCellularGridSpace KPreSpace;
  typedef typename KSpace::Integer Integer;
  typedef typename KSpace::Sign Sign;
  typedef CellDeltaSelector<TKSpace> CellDelta;

  struct Row {
    char cardinalDir;
    Sign sign;
    CellDelta delta;
    Row() = default;
    Row(const Row& other) = default;
    Row(Row&& other) = default;
    Row(char aCardinalDir, Sign aSign, CellDelta aDelta) : cardinalDir(aCardinalDir), sign(aSign), delta(aDelta) {}
    void selfDisplay(std::ostream &out) const {
      out << "[SignTableRow " << cardinalDir << ": " << sign << ", " << delta << " ]";
    }
  };

private:
  typedef KPreSpace::Cell Cell;
  typedef KPreSpace::SCell SCell;
  typedef KPreSpace::Point Point;
  static std::map<char, Row> buildMap() {
    std::map<char, Row> result;

    Cell interiorSpel = KPreSpace::uSpel(Point(0, 0));
    Point interiorKCoords = KPreSpace::uKCoords(interiorSpel);
    auto uSurfels = KPreSpace::uLowerIncident(interiorSpel);

    std::vector<SCell> sSurfels;
    for (auto uSurfel : uSurfels) {
      SCell sSurfel = KPreSpace::sCell(KPreSpace::uKCoords(uSurfel), true); // arbitrary initial sign
      SCell direct = KPreSpace::sDirectIncident(sSurfel, KPreSpace::sOrthDir(sSurfel));
      if (KPreSpace::sKCoords(direct) == interiorKCoords) {
        KPreSpace::sSetSign(sSurfel, true);
        sSurfels.push_back(sSurfel);
        continue;
      }
      SCell indirect = KPreSpace::sIndirectIncident(sSurfel, KPreSpace::sOrthDir(sSurfel));
      if (KPreSpace::sKCoords(indirect) == interiorKCoords) {
        KPreSpace::sSetSign(sSurfel, false);
        sSurfels.push_back(sSurfel);
        continue;
      }
      ASSERT_MSG(false, "Shouldn't be here!");
    }

    for (auto surfel : sSurfels) {
      DGtal::Dimension d = *KPreSpace::sDirs(surfel);
      Point surfelHead = KPreSpace::sKCoords(KPreSpace::sDirectIncident(surfel, d));
      Point surfelTail = KPreSpace::sKCoords(KPreSpace::sIndirectIncident(surfel, d));
      CellDelta cd{d, (surfelHead - surfelTail)[d]};
      ASSERT_MSG(cd.isValid(), "invalid delta");
      ASSERT_MSG(cd.abs() == 2, "delta.abs() wasn't 2");

      char key;
      if (cd.dim == 0) {
        key = (cd.delta > 0) ? 'E' : 'W';
      }
      else {
        key = (cd.delta > 0) ? 'N' : 'S';
      }
      ASSERT_MSG(!result.contains(key), "duplicate map entry somehow") ;
      result.insert(std::make_pair(key, Row(key, KPreSpace::sSign(surfel), cd)));
    }
    return result;
  }

  static inline std::map<char, Row> rows = buildMap();

public:
  static inline const Row N = rows['N'];
  static inline const Row E = rows['E'];
  static inline const Row S = rows['S'];
  static inline const Row W = rows['W'];

  static Sign getSign(char cardinalDir) {
    return getRow(cardinalDir).sign;
  }
  static Sign getSign(const CellDelta& delta) {
    return getRow(delta).sign;
  }

  static char getChar(char cardinalDir) {
    return getRow(cardinalDir).cardinalDir;
  }
  static char getChar(const CellDelta& delta) {
    return getRow(delta).cardinalDir;
  }

  static CellDelta getDelta(char cardinalDir, Integer length) {
    auto d = getRow(cardinalDir).delta;
    return CellDelta{d.dim, d.delta * length};
  }

private:
  static Row getRow(const CellDelta& delta) {
    ASSERT_MSG(delta.isValid(), "supplied delta was invalid");
    if (delta.dim == 0) {
      return (delta.delta > 0) ? E : W;
    }
    return (delta.delta > 0) ? N : S;
  }
  
  static Row getRow(char cardinalDir) {
    switch (cardinalDir) {
      case 'N': return N;
      case 'E': return E;
      case 'S': return S;
      case 'W': return W;
    }
    ASSERT_MSG(false, "cardinalDir must be one of N, E, S, W");
    return N;
  }
};

} // end namespace detail

template <concepts::CXCellularGridSpace2D TKSpace>
std::ostream& operator<<(std ::ostream &out, const typename detail::SignTable<TKSpace>::Row &object) {
  object.selfDisplay(out);
  return out;
}

} // end namespace DGtalTrace