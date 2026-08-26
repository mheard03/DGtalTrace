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

namespace DGtalTrace {
namespace detail {

/*
namespace {
  template <typename T>
  requires concepts::CHasDimension<T>
  using Topology = std::bitset<DimensionOf<T>>;
  
  template <concepts::CXXCell TCell>
  Topology<TCell> cellTopology(const TCell &cell) {
    static constexpr DGtal::Dimension dim = DimensionOf<TCell>;
    typedef KCoordsAccessor<typename CellTypeTraits<TCell>::KPreSpace> KCA;
    typedef DGtal::NumberTraits<typename TCell::Integer> NT;

    Topology<TCell> result;
    for (DGtal::Dimension k = 0; k < dim; ++k) {
      result.set(k, NT::odd(KCA::kCoord(cell, k)));
    }
  }

  template <concepts::CXCellularGridSpaceND TKSpace>
  constexpr std::array<typename TKSpace::Sign, 1LL << TKSpace::dimension> topologySignFlips;
}
*/



// using TKSpace = DGtal::Z2i::KSpace;
template <concepts::CXCellularGridSpaceND TKSpace>
class CellDeltaHelper {
public:
  typedef TKSpace KSpace;
  typedef KSpace::PreCellularGridSpace KPreSpace;
  typedef KCoordsAccessor<KSpace> KCA;
  typedef KSpace::Vector Vector;
  typedef KSpace::Sign Sign;
  typedef KSpace::Integer Integer;
  typedef Vector::UnsignedComponent UInteger;
  using CellDelta = CellDeltaSelector<KSpace>;

public: // <-- TODO: Private
  static inline constexpr bool isPreSpace = concepts::CPreCellularGridSpaceND<KSpace>;
  typedef DGtal::NumberTraits<Integer> NT;

  struct RawDelta {
    DGtal::Dimension dim;
    Integer start;
    Integer end;
    UInteger expectedAbs;

    inline Integer delta() const { 
      return end - start;
    };
    inline Integer abs() const {
      return std::abs(delta());
    }
    inline bool isValid() const {
      return expectedAbs > 0u && expectedAbs <= 2u && abs() == static_cast<Integer>(expectedAbs);
    }
    operator Vector() {
      return static_cast<Vector>(this->operator CellDelta());
    }

    inline operator CellDelta() {
      ASSERT_MSG(isValid(), "Tried to cast invalid RawDelta to Delta");
      return CellDelta{dim, delta()};
    }
    void selfDisplay(std::ostream &out) const {
      out << "[Raw Delta " << delta() << " on dim " << dim;
      if (!isValid()) {
        out << "; range [" << start << "->" << end << "], expected "
            << expectedAbs;
      }
      out << "]";
    }
  };

  template <concepts::CXXCell TCell1, concepts::CXXCell TCell2>
  static RawDelta getRawDelta(const TCell1 &startCell, const TCell2 &endCell) {
    std::optional<DGtal::Dimension> dim;
    Integer start = 0, end = 0, startDim = 0, endDim = 0;
    for (DGtal::Dimension k = 0; k < KPreSpace::dimension; k++) {
      Integer startK = KCA::kCoord(startCell, k), endK = KCA::kCoord(endCell, k);
      startDim += (NT::odd(startK) ? 1 : 0);
      endDim += (NT::odd(endK) ? 1 : 0);
      if (startK == endK)
        continue;
      ASSERT_MSG(dim == std::nullopt, "Difference between cell dimensions can be at most 1");
      dim = k, start = startK, end = endK;
    }
    ASSERT_MSG(dim.has_value(), "Cells had equal kCoords");

    Integer dimDiff = std::abs(startDim - endDim);
    ASSERT_MSG(dimDiff <= 1, "Difference between cell dimensions can be at most 1");
    UInteger expectedMagnitude = (dimDiff == 0) ? 2 : 1;
    return RawDelta{dim.value(), start, end, expectedMagnitude};
  }

public:
  CellDeltaHelper(DGtal::ConstAlias<KSpace> ks) : myKPtr(&ks) {}

  CellDeltaHelper()
    requires(isPreSpace)
      : CellDeltaHelper(preSpaces<KSpace>) {}

  template <concepts::CXXCell TCell1, concepts::CXXCell TCell2>
    requires(isPreSpace)
  static CellDelta getDelta(const TCell1 &startCell, const TCell2 &endCell) {
    return static_cast<CellDelta>(getRawDelta(startCell, endCell));
  }

  template <concepts::CXXCell TCell1, concepts::CXXCell TCell2>
    requires(!isPreSpace)
  CellDelta getDelta(const TCell1 &startCell, const TCell2 &endCell) const {
    ASSERT_MSG(myKPtr->cIsValid(KCA::kCoords(startCell)), "startCell was invalid");
    ASSERT_MSG(myKPtr->cIsValid(KCA::kCoords(endCell)), "endCell was invalid");
    RawDelta raw = getRawDelta(startCell, endCell);

    if (!raw.isValid() && myKPtr->isSpacePeriodic(raw.dim)) [[unlikely]] {
      // If we got an invalid delta but the space is periodic, double-check to
      // make sure it's not just a magnitude issue due to wraparound
      typename KSpace::Cell shifted = myKPtr->uCell(KCA::kCoords(startCell));
      for (Integer delta : { static_cast<Integer>(raw.expectedAbs), -static_cast<Integer>(raw.expectedAbs) }) {
        myKPtr->uSetKCoord(shifted, raw.dim, KCA::kCoord(startCell, raw.dim) + delta);
        if (KCA::kCoord(shifted, raw.dim) == KCA::kCoord(endCell, raw.dim)) {
          return CellDelta{raw.dim, delta};
        }
      }
    }

    return raw;
  }

  static CellDelta getDelta(const KPreSpace::SCell aSurfel,
                            const Integer aDelta) {
    ASSERT_MSG(KCA::isSurfel(aSurfel), "supplied cell was not a surfel");
    auto dim = *KPreSpace::sDirs(aSurfel);
    return CellDelta{dim, aDelta};
  }

  static CellDelta getInwardDelta(const KPreSpace::Surfel aSurfel) {
    ASSERT_MSG(KCA::isSurfel(aSurfel), "supplied cell was not a surfel");
    auto dim = KPreSpace::sOrthDir(aSurfel);
    auto voxel = KPreSpace::sIncident(aSurfel, dim, KPreSpace::sDirect(aSurfel, dim));
    auto delta = KCA::kCoords(voxel)[dim] - KCA::kCoords(aSurfel)[dim];
    ASSERT_MSG(delta == 1 || delta == -1, "wiggity-wack");
    return CellDelta{dim, delta};
  }

private:
  const KSpace *myKPtr;
  typedef typename KSpace::Cell Cell;
  typedef typename KSpace::Point Point;
};

} // end namespace detail

FN_STREAM_SELFDISPLAY_T(typename DGtalTrace::detail::CellDeltaHelper<T>::Row)

} // end namespace DGtalTrace