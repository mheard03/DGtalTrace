#pragma once
#include <DGtal/base/Assert.h>
#include <DGtal/base/Common.h>
#include <DGtal/base/ConstAlias.h>
#include <DGtal/kernel/NumberTraits.h>
#include "../../concepts/BaseConcepts.cpp"
#include "../../concepts/CCell.cpp"
#include "../../concepts/CXCellularGridSpaceND.cpp"
#include "../Globals.cpp"
#include "../CellTypeTraits.cpp"
#include "CellDelta.cpp"
#include "../KCoordsAccessor.cpp"


namespace DGtalTrace {
namespace detail {

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
  typedef CellDelta<KSpace> CellDelta;

public:  // <-- TODO: Private
  static inline constexpr bool isPreSpace = concepts::CPreCellularGridSpaceND<KSpace>;
  using Magnitude = CellDelta::Magnitude;

  struct RawDelta {
    DGtal::Dimension dim;
    Integer start;
    Integer end;    
    Magnitude expectedMagnitude;
    inline Integer mag() const {
      return std::abs(smag());
    };
    inline Sign sign() const {
      return (smag() >= 0) ? KSpace::POS : KSpace::NEG;
    };
    inline bool isValid() const {
      return start != end && mag() == static_cast<UInteger>(expectedMagnitude);
    }
    inline Integer smag() const {
      return end - start;
    }
    inline operator CellDelta() {
      ASSERT_MSG(isValid(), "Tried to cast invalid RawDelta to Delta");
      return CellDelta{dim, expectedMagnitude, sign()};
    }
    void selfDisplay(std::ostream &out) const {
      out << "[Raw Delta " << smag() << " on dim " << dim;
      if (!isValid()) {
        out << "; range [" << start << "->" << end << "], expected " << expectedMagnitude;
      }
      out << "]";
    }
  };

  template <concepts::CXXCell TCell1, concepts::CXXCell TCell2>
  static RawDelta getRawDelta(const TCell1& startCell, const TCell2& endCell) {
    std::optional<DGtal::Dimension> dim;
    Integer start = 0, end = 0, startDim = 0, endDim = 0;
    for (DGtal::Dimension k = 0; k < 2; k++) {
      Integer startK = KCA::kCoord(startCell, k), endK = KCA::kCoord(endCell, k);
      startDim += (DGtal::NumberTraits<Integer>::odd(startK) ? 1 : 0);
      endDim += (DGtal::NumberTraits<Integer>::odd(endK) ? 1 : 0);
      if (startK == endK) continue;
      ASSERT_MSG(dim == std::nullopt, "Difference between cell dimensions can be at most 1");
      dim = k, start = startK, end = endK;
    }
    ASSERT_MSG(dim.has_value(), "Cells had equal kCoords");

    Integer dimDiff = std::abs(startDim - endDim);
    ASSERT_MSG(dimDiff <= 1, "Difference between cell dimensions can be at most 1");
    Magnitude expectedMagnitude = (dimDiff == 0) ? Magnitude::TWO : Magnitude::ONE;
    return RawDelta{dim.value(), start, end, expectedMagnitude};
  }

public:
  CellDeltaHelper(DGtal::ConstAlias<KSpace> ks)
    : myKPtr(&ks) {}

  CellDeltaHelper() requires (isPreSpace)
    : CellDeltaHelper(preSpaces<KSpace>) {}

  template <concepts::CXXCell TCell1, concepts::CXXCell TCell2>
  requires (isPreSpace)
  static CellDelta getDelta(const TCell1& startCell, const TCell2& endCell) {
    return getRawDelta(startCell, endCell);
  }

  template <concepts::CXXCell TCell1, concepts::CXXCell TCell2>
  requires (!isPreSpace)
  CellDelta getDelta(const TCell1& startCell, const TCell2& endCell) const {
    ASSERT_MSG(myKPtr->cIsValid(KCA::kCoords(startCell)), "startCell was invalid");
    ASSERT_MSG(myKPtr->cIsValid(KCA::kCoords(endCell)), "endCell was invalid");
    RawDelta raw = getRawDelta(startCell, endCell);
    
    if (!raw.isValid()) { // } && myKPtr->isSpacePeriodic(raw.dim)) [[unlikely]] {
      // If we got an invalid delta but the space is periodic, double-check to make sure
      // it's not just a magnitude issue due to wraparound
      typedef typename KSpace::Cell Cell;
      Cell shifted = myKPtr->uCell(KCA::kCoords(startCell));

      // shift positive and check
      myKPtr->uSetKCoord(shifted, raw.dim, KCA::kCoord(startCell, raw.dim) + raw.expectedMagnitude);
      // DGtal::trace.info() << std::endl << "  trying shift " << startCell << " -> " << shifted << " vs. " << endCell << std::endl;
      if (KCA::kCoord(shifted, raw.dim) == KCA::kCoord(endCell, raw.dim)) {
        return CellDelta{raw.dim, raw.expectedMagnitude, KSpace::POS};
      }

      // shift negative and check
      myKPtr->uSetKCoord(shifted, raw.dim, KCA::kCoord(startCell, raw.dim) - raw.expectedMagnitude);
      // DGtal::trace.info() << std::endl << "  trying shift " << startCell << " -> " << shifted << " vs. " << endCell << std::endl;
      if (KCA::kCoord(shifted, raw.dim) == KCA::kCoord(endCell, raw.dim)) {
        return CellDelta{raw.dim, raw.expectedMagnitude, KSpace::NEG};
      }
    }

    return raw;
  }

private:
  const KSpace* myKPtr;
  typedef typename KSpace::Cell Cell;
  typedef typename KSpace::Point Point;

};

} // end namespace detail
} // end namespace DGtalTrace

