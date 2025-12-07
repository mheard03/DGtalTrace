#pragma once
#include <DGtal/base/Common.h>
#include <array>
#include <utility>
#include <type_traits>
#include <DGtal/helpers/StdDefs.h>
#include <DGtal/base/Trace.h>
#include "traceHelper.cpp"

template <typename T, typename = void>
struct is_cell : std::false_type {};

template <typename T>
struct is_cell<T, std::void_t<typename T::CellularGridSpace, typename T::PreCellularGridSpace>> : std::disjunction<
  std::is_same<T, typename T::CellularGridSpace::Cell>,
  std::is_same<T, typename T::CellularGridSpace::SCell>,
  std::is_same<T, typename T::PreCellularGridSpace::Cell>,
  std::is_same<T, typename T::PreCellularGridSpace::SCell>
> {};

template <typename T, typename = void>
struct is_precell : std::false_type {};

template <typename T>
struct is_precell<T, std::void_t<typename T::CellularGridSpace, typename T::PreCellularGridSpace>> : std::disjunction<
  std::is_same<T, typename T::PreCellularGridSpace::Cell>,
  std::is_same<T, typename T::PreCellularGridSpace::SCell>
> {};

template <typename T, typename = void>
struct is_scell : std::false_type {};

template <typename T>
struct is_scell<T, std::void_t<typename T::CellularGridSpace, typename T::PreCellularGridSpace>> : std::disjunction<
  std::is_same<T, typename T::CellularGridSpace::SCell>,
  std::is_same<T, typename T::PreCellularGridSpace::SCell>
> {};

template <typename TCell, typename = void>
struct CellPropsBase {};

template <typename TCell>
struct CellPropsBase<TCell, typename std::enable_if_t<is_cell<TCell>::value>> {
  typedef DGtal::Z2i::KSpace TKSpace;
  // typedef typename TCell::CellularGridSpace TKSpace;
  typedef typename TKSpace::Cell KCell;
  typedef typename TKSpace::SCell KSCell;
  typedef TKSpace::PreCellularGridSpace TKPreSpace;
  // typedef typename TCell::PreCellularGridSpace TKPreSpace;
  typedef typename TKPreSpace::Cell KPCell;
  typedef typename TKPreSpace::SCell KPSCell;

  static const TKSpace dK;

  static constexpr bool isSigned = std::is_same_v<TCell, KSCell> || std::is_same_v<TCell, KPSCell>;
  static constexpr bool isPrecell = std::is_same_v<TCell, KPCell> || std::is_same_v<TCell, KPSCell>;

  template <typename TTCell, std::enable_if_t<is_cell<TTCell>::value> = true>

  static KCell stripSign(const KCell& cell) { return cell; };
  static KCell stripSign(const KSCell& cell) { return dK.unsigns(cell); };
  static KPCell stripSign(const KPCell& cell) { return cell; };
  static KPCell stripSign(const KPSCell& cell) { return TKPreSpace::unsigns(cell); };

  /*
  static DGtal::Dimension dim(const KCell& cell) { return dK.uDim(cell); };
  static DGtal::Dimension dim(const KSCell& cell) { return dK.unsigns(cell); };
  static DGtal::Dimension dim(const KPCell& cell) { return cell; };
  static DGtal::Dimension dim(const KPSCell& cell) { return TKPreSpace::unsigns(cell); };
  */

  template <typename TTCell, std::enable_if_t<is_cell<TTCell>::value> = true>
  static DGtal::Dimension dim(TTCell& cell) {
    auto raw = stripSign(cell.preCell());
    return TKPreSpace::uDim(raw);
  }
};

template <typename TCell, bool isSigned, bool isPrecell, typename = void>
struct CellFuncs {};

template <typename TCell>
struct CellFuncs<TCell, true, false, typename std::enable_if_t<is_scell<TCell>::value && !is_precell<TCell>::value>> {
  static typename TCell::CellularGridSpace tempK;
  
  static typename TCell::CellularGridSpace::Cell stripSign(const TCell& cell) { return tempK.unsigns(cell); }
  typedef UCell = TCell::Cell
  
  


  typedef DGtal::Z2i::KSpace TKSpace;
  // typedef typename TCell::CellularGridSpace TKSpace;
  typedef typename TKSpace::Cell KCell;
  typedef typename TKSpace::SCell KSCell;
  typedef TKSpace::PreCellularGridSpace TKPreSpace;
  // typedef typename TCell::PreCellularGridSpace TKPreSpace;
  typedef typename TKPreSpace::Cell KPCell;
  typedef typename TKPreSpace::SCell KPSCell;
};

template <bool isSigned, bool isPrecell, typename TCell, typename = void>
struct CellFuncs {};

template <typename TCell, bool isSigned = is_scell<TCell>::value, bool isPrecell = is_precell<TCell>::value>
struct CellProps : CellPropsBase<TCell> {};

template <>
struct CellProps<DGtal::Z2i::KSpace::Cell : CellPropsBase<TCell, typename std::enable_if_t<is_cell<TCell>::value>> {
  typedef DGtal::Z2i::KSpace TKSpace;
  // typedef typename TCell::CellularGridSpace TKSpace;
  typedef typename TKSpace::Cell KCell;
  typedef typename TKSpace::SCell KSCell;
  typedef TKSpace::PreCellularGridSpace TKPreSpace;
  // typedef typename TCell::PreCellularGridSpace TKPreSpace;
  typedef typename TKPreSpace::Cell KPCell;
  typedef typename TKPreSpace::SCell KPSCell;

  static const TKSpace dK;

  static constexpr bool isSigned = std::is_same_v<TCell, KSCell> || std::is_same_v<TCell, KPSCell>;
  static constexpr bool isPrecell = std::is_same_v<TCell, KPCell> || std::is_same_v<TCell, KPSCell>;

  template <typename TTCell, std::enable_if_t<is_cell<TTCell>::value> = true>

  static KCell stripSign(const KCell& cell) { return cell; };
  static KCell stripSign(const KSCell& cell) { return dK.unsigns(cell); };
  static KPCell stripSign(const KPCell& cell) { return cell; };
  static KPCell stripSign(const KPSCell& cell) { return TKPreSpace::unsigns(cell); };

  static DGtal::Dimension dim(const KCell& cell) { return dK.uDim(cell); };
  static DGtal::Dimension dim(const KSCell& cell) { return dK.unsigns(cell); };
  static DGtal::Dimension dim(const KPCell& cell) { return cell; };
  static DGtal::Dimension dim(const KPSCell& cell) { return TKPreSpace::unsigns(cell); };

  template <typename TTCell, std::enable_if_t<is_cell<TTCell>::value> = true>
  static DGtal::Dimension dim(TTCell& cell) {
    auto raw = stripSign(cell.preCell());
    return TKPreSpace::uDim(raw);
  }
};


typedef CellPropsBase<DGtal::Z2i::Cell> cp;
DGtal::Z2i::Cell c;
auto i = cp::dim(c);


// template <typename TKSpace>
struct XCell {
  typedef DGtal::Z2i::KSpace TKSpace;

  typedef TKSpace::PreCellularGridSpace PK;
  typedef TKSpace::Cell Cell;
  typedef TKSpace::SCell SCell;
  typedef TKSpace::PreCell PreCell;
  typedef TKSpace::SPreCell SPreCell;

  template <typename TCell, std::enable_if_t<is_cell<TCell>::value, bool> = true>
  static DGtal::Dimension dim(TCell& cell) {
    return PK::sDim(cell);
  }

};

DGtal::Z2i::Cell c;
XCell::


/*
template <typename T, typename = void>
struct is_cell : std::false_type {};

template <typename T>
struct is_cell<T, std::void_t<typename T::CellularGridSpace>> : std::disjunction<
  std::is_same<T, typename T::CellularGridSpace::Cell>,
  std::is_same<T, typename T::CellularGridSpace::SCell>
> {};

template <typename T, typename = void>
struct is_precell : std::false_type {};

template <typename T>
struct is_precell<T, std::void_t<typename T::PreCellularGridSpace>> : std::disjunction<
  std::is_same<T, typename T::PreCellularGridSpace::Cell>,
  std::is_same<T, typename T::PreCellularGridSpace::SCell>
> {};

template <typename T, typename = void>
struct is_any_cell : std::false_type {};

template <typename T>
struct is_any_cell<T> : std::disjunction<is_cell<T>, is_precell<T>> {};
*/