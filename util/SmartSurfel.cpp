#pragma once
#include <DGtal/base/Assert.h>
#include <cstdint>
#include <ranges>
#include <vector>
#include <type_traits>

#include <DGtal/base/Common.h>
#include <DGtal/helpers/StdDefs.h> // <- delete this
#include <DGtal/kernel/PointVector.h>
#include <DGtal/kernel/NumberTraits.h>

#include "../concepts/CCell.cpp"
#include "../concepts/CXCellularGridSpaceND.cpp"
#include "../util/CellTypeTraits.cpp"
#include "../util/Globals.cpp"
#include "DimensionOf.cpp"



namespace DGtalTrace {

template <concepts::CXXCell TCell>
struct SmartCell;

namespace {
  template <class T> struct is_smart_cell : std::false_type {};
  template <concepts::CXXCell TCell> struct is_smart_cell<SmartCell<TCell>> : std::true_type {};
} // end anonymous namespace

template <typename T>
concept IsSmartCell = is_smart_cell<T>::value;

struct CellConverter {
  #define TFromAndTTo template <concepts::CXXCell TFrom, concepts::CXXCell TTo>

  template <concepts::CXXCell TCell>
  static inline constexpr bool isSmart = is_smart_cell<TCell>::value;

  template <concepts::CXXCell TCell>
  static inline constexpr bool isSigned = concepts::CSXCell<TCell>;
  
  template <concepts::CXXCell TCell>
  static inline constexpr bool isFull = concepts::CXCell<TCell>;

  template <concepts::CXXCell TCell>
  using KPreSpace = TCell::PreCellularGridSpace;

  template <concepts::CSXCell TCell>
  inline static auto dropSign(const TCell& cell) {
    if (!isSigned<TCell>) {
      return cell;
    }
    else if (!isFull<TCell>) {
      return KPreSpace<TCell>::unsigns(cell); 
    }
    else {
      typedef typename TCell::CellularGridSpace KSpace;
      const KSpace* ksInfinite = &detail::infiniteKSpaces<typename TCell::CellularGridSpace>;
      return ksInfinite->unsigns(cell);
    }
  }

  TFromAndTTo
  static inline constexpr bool canOperate = std::same_as<KPreSpace<TFrom>, KPreSpace<TTo>>;


  // -- cast function  --------------------------------------------------------
  TFromAndTTo
  requires canOperate<TFrom, TTo> &&
    (isFull<TFrom> >= isFull<TTo>) &&
    (isSigned<TFrom> == isSigned<TTo>)
  inline static TTo cast(const TFrom& source) {
    return static_cast<TTo>(source);
  }

  TFromAndTTo
  static inline constexpr bool canCast = requires (TFrom source) { cast(source); };


  // -- convert function  -----------------------------------------------------
  TFromAndTTo
  requires canOperate<TFrom, TTo> &&
    (isFull<TFrom> >= isFull<TTo>) &&
    (isSigned<TFrom> >= isSigned<TTo>)  // <- This is the only difference from cast()
  inline static TTo convert(const TFrom& source) {
    if (canCast<TFrom, TTo>) {
      return cast<TTo>(source);
    }
    else {
      return cast<TTo>(dropSign(source));
    }
  }

  TFromAndTTo
  static inline constexpr bool canConvert = requires (TFrom source) { convert(source); };


  // -- convertUnsafe function  -----------------------------------------------
  template <concepts::CXXCell TFrom, concepts::CXXCell TTo>
  requires canOperate<TFrom, TTo> &&
    // (isFull<TFrom> >= isFull<TTo>) &&   // <- This is the only difference from convert
    (isSigned<TFrom> >= isSigned<TTo>)
  inline static TTo convertUnsafe(const TFrom& source) {
    if constexpr (canConvert<TFrom, TTo>) return convert<TTo>(source);

    // known: casting pre->full, and possibly signed->unsigned=
    if (isSigned<TFrom> == isSigned<TTo>) throw "fkn how?";
    
    typedef typename TFrom::CellularGridSpace FromSpace;
    if constexpr (isFull<TFrom>) {
      const FromSpace* ks = &detail::infiniteKSpaces<typename TFrom::CellularGridSpace>;
      return cast<TTo>(ks->unsigns(source));      
    }
    else {
      return cast<TTo>(KPreSpace<TFrom>::unsigns(source)); 
    }
  }
  #undef TFromAndTTo
};


template <concepts::CXXCell TCell>
struct SmartCell : public TCell {
  friend struct CellConverter;
private:  
  typedef CellTypeTraits<TCell> Traits;
  typedef typename Traits::KPreSpace KPreSpace;
  typedef typename Traits::SSelf SSelf;
  typedef typename Traits::USelf USelf;
  typedef typename Traits::SSelf SPreSelf;
  typedef typename Traits::USelf UPreSelf;
  #define isSigned concepts::CSXCell<TCell>

public:
  using CellularGridSpace = TCell::CellularGridSpace;

  using TCell::TCell;
  using TCell::operator=;
  inline static DGtal::Dimension dimension = KPreSpace::dimension;

  inline DGtal::Dimension dim() {
    return (isSigned) ? KPreSpace::sDim(*this) : KPreSpace::uDim(*this);
  }
  inline KPreSpace::Integer topology() {
    return (isSigned) ? KPreSpace::sTopology(*this) : KPreSpace::uTopology(*this);
  }
  inline KPreSpace::Point kCoords() {
    return (isSigned) ? KPreSpace::sKCoords(*this) : KPreSpace::uKCoords(*this);
  }

  inline KPreSpace::Sign sign() 
  requires (isSigned) {
    return KPreSpace::sSign(*this);
  }

  #undef isSigned
};

void init() {
  typedef DGtal::Z2i::KSpace::Surfel Surfel;
  typedef DGtal::Z2i::KSpace::Cell Cell;
  typedef SmartCell<Surfel> SmartSurfel;
  typedef SmartCell<Cell> SmartUCell;
  SmartSurfel ss;
  SmartUCell su;
  Surfel s;
  ss = s;
  s = ss;
  ss = su;
  su = ss;

}


typedef typename DGtal::Z2i::KPreSpace TKPreSpace;
typedef typename DGtal::Z2i::KSpace TKSpace;



// template <concepts::CPreCellularGridSpaceND TKPreSpace>
// requires (DimensionOf<TKPreSpace> == 2)
struct SmartSurfel : public SmartXCell<TKPreSpace::Surfel> {
private:
  typedef typename TKPreSpace::Surfel Base;
  typedef DGtal::Dimension Dimension;

public:
  typedef TKPreSpace KPreSpace;
  typedef KPreSpace::Point Point;
  typedef KPreSpace::Integer Integer;
  typedef typename KPreSpace::SCell SPreCell;
  typedef typename KPreSpace::Cell PreCell;
  typedef SPreCell Surfel;  
  typedef PreCell Pointel;
  typedef PreCell Spel;
  inline static DGtal::Dimension dimension = KPreSpace::dimension;

  using Base::Base;
  using Base::operator=;
  explicit SmartSurfel(Integer dummy = 0) : Base(0) {}
  explicit SmartSurfel(SPreCell const& aSurfel) : Base(aSurfel) {
    ASSERT_MSG(KPreSpace::sDim(aSurfel) == 1, "Tried to construct SmartSurfel from sCell with wrong dimension.");
  }
  explicit SmartSurfel(SPreCell&& aSurfel) : Base(aSurfel) {
    ASSERT_MSG(KPreSpace::sDim(aSurfel) == 1, "Tried to construct SmartSurfel from sCell with wrong dimension.");
  }

  Pointel head() const {
    return KPreSpace::unsigns(KPreSpace::sIndirectIncident(*this, *KPreSpace::sDirs(*this)));
  }
  Pointel tail() const {
    return KPreSpace::unsigns(KPreSpace::sDirectIncident(*this, *KPreSpace::sDirs(*this)));
  }

  Spel interiorVoxel( const SCell & c ) ;

    /** @brief For a given surfel ((n-1)-signed cell), returns its exterior voxel
     * (point in Z^d given by the indirect incident n-cell to the given cell).
     *
     * @param c any signed pre-cell.
     * @return the outer voxel.
     */
    static Point exteriorVoxel( const SCell & c ) ;
};

// template <concepts::CPreCellularGridSpaceND TKPreSpace>
// requires (DimensionOf<TKPreSpace> == 2)
struct SmartSurfel : public TKPreSpace::Surfel {
private:
  typedef typename TKPreSpace::Surfel Base;
  typedef DGtal::Dimension Dimension;

public:
  typedef TKPreSpace KPreSpace;
  typedef KPreSpace::Point Point;
  typedef KPreSpace::Integer Integer;
  typedef typename KPreSpace::SCell SPreCell;
  typedef typename KPreSpace::Cell PreCell;
  typedef SPreCell Surfel;  
  typedef PreCell Pointel;
  typedef PreCell Spel;
  inline static DGtal::Dimension dimension = KPreSpace::dimension;

  using Base::Base;
  using Base::operator=;
  explicit SmartSurfel(Integer dummy = 0) : Base(0) {}
  explicit SmartSurfel(SPreCell const& aSurfel) : Base(aSurfel) {
    ASSERT_MSG(KPreSpace::sDim(aSurfel) == 1, "Tried to construct SmartSurfel from sCell with wrong dimension.");
  }
  explicit SmartSurfel(SPreCell&& aSurfel) : Base(aSurfel) {
    ASSERT_MSG(KPreSpace::sDim(aSurfel) == 1, "Tried to construct SmartSurfel from sCell with wrong dimension.");
  }

  Pointel head() const {
    return KPreSpace::unsigns(KPreSpace::sIndirectIncident(*this, *KPreSpace::sDirs(*this)));
  }
  Pointel tail() const {
    return KPreSpace::unsigns(KPreSpace::sDirectIncident(*this, *KPreSpace::sDirs(*this)));
  }

  Spel interiorVoxel( const SCell & c ) ;

    /** @brief For a given surfel ((n-1)-signed cell), returns its exterior voxel
     * (point in Z^d given by the indirect incident n-cell to the given cell).
     *
     * @param c any signed pre-cell.
     * @return the outer voxel.
     */
    static Point exteriorVoxel( const SCell & c ) ;
};


void init() {
  typedef SmartSurfel::Surfel Surfel;
  SmartSurfel ss()
  Surfel s;
  ss = s;
}

}