#pragma once
#include <iostream>
#include <unordered_set>
#include <unordered_map>
#include <DGtal/base/Common.h>
#include <DGtal/kernel/CInteger.h>
#include <DGtal/kernel/PointVector.h>
#include <DGtal/kernel/SpaceND.h>
#include <DGtal/topology/KhalimskyPreSpaceND.h>

 // 
namespace DGtalTrace {

// Pre-declaration
template <DGtal::Dimension dim, typename TInteger = DGtal::int32_t>
class KhalimskyPreSpaceND;

// PreCell
template <DGtal::Dimension dim, typename TInteger = DGtal::int32_t>
struct KhalimskyPreCell : public DGtal::KhalimskyPreCell<dim, TInteger> {
  typedef DGtal::KhalimskyPreCell<dim, TInteger> Base;
 public:
  using Integer = Base::Integer;
  using UnsignedInteger = Base::UnsignedInteger;
  using Point = Base::Point;
  using Self = KhalimskyPreCell<dim, Integer>;
  using PreCell = Self;
  using PreCellularGridSpace = KhalimskyPreSpaceND<dim, TInteger>;
  using CellularGridSpace = PreCellularGridSpace;

  explicit KhalimskyPreCell(Integer dummy = 0) : Base(dummy) {}
  KhalimskyPreCell( Point const& aPoint, bool positive ) : Base (aPoint, positive) {}
  KhalimskyPreCell( KhalimskyPreCell const& aCell ) = default;
  KhalimskyPreCell & operator=( KhalimskyPreCell const& aCell ) = default;
  KhalimskyPreCell( KhalimskyPreCell && aCell ) = default;
  KhalimskyPreCell & operator=( KhalimskyPreCell && aCell ) = default;
}; // KhalimskyPreCell


// SPreCell
template <DGtal::Dimension dim, typename TInteger = DGtal::int32_t>
struct SignedKhalimskyPreCell : public DGtal::SignedKhalimskyPreCell<dim, TInteger> {
  typedef DGtal::SignedKhalimskyPreCell<dim, TInteger> Base;
 public:
  using Integer = Base::Integer;
  using UnsignedInteger = Base::UnsignedInteger;
  using Point = Base::Point;
  using Self = SignedKhalimskyPreCell<dim, Integer>;
  using SPreCell = Self;
  using PreCellularGridSpace = KhalimskyPreSpaceND<dim, TInteger>;
  using CellularGridSpace = PreCellularGridSpace;

  SPreCell const& preCell() const {
    return *this;
  }

  explicit SignedKhalimskyPreCell(Integer dummy = 0) : Base(dummy) {}
  SignedKhalimskyPreCell( Point const& aPoint, bool positive ) : Base (aPoint, positive) {}
  SignedKhalimskyPreCell( SignedKhalimskyPreCell const& aCell ) = default;
  SignedKhalimskyPreCell & operator=( SignedKhalimskyPreCell const& aCell ) = default;
  SignedKhalimskyPreCell( SignedKhalimskyPreCell && aCell ) = default;
  SignedKhalimskyPreCell & operator=( SignedKhalimskyPreCell && aCell ) = default;
}; // SignedKhalimskyPreCell


// PreCellDirectionIterator
template <DGtal::Dimension dim, typename TInteger = DGtal::int32_t>
class PreCellDirectionIterator : public DGtal::PreCellDirectionIterator<dim, TInteger> {
  typedef DGtal::PreCellDirectionIterator<dim, TInteger> Base;
 public:
  using Integer = Base::Integer;
  typedef KhalimskyPreCell<dim, Integer> Cell;
  typedef SignedKhalimskyPreCell<dim, Integer> SCell;

  explicit PreCellDirectionIterator(Cell cell, bool open = true) : Base(static_cast<DGtal::KhalimskyPreCell<dim, Integer>>(cell), open) {}
  explicit PreCellDirectionIterator(SCell cell, bool open = true) : Base(static_cast<DGtal::SignedKhalimskyPreCell<dim, Integer>>(cell), open) {}
}; // PreCellDirectionIterator


template <DGtal::Dimension dim, typename TInteger>
class KhalimskyPreSpaceND : DGtal::KhalimskyPreSpaceND<dim, TInteger> {
  typedef DGtal::KhalimskyPreSpaceND<dim, TInteger> Base;
  typedef KhalimskyPreSpaceND<dim, TInteger> Self;
 public:
  using Integer = Base::Integer;
  using Cell    = KhalimskyPreCell< dim, Integer >;
  using SCell   = SignedKhalimskyPreCell< dim, Integer >;
  using Surfel  = SCell;
  using Sign    = Base::Sign;
  using Point   = Base::Point;
  using Vector  = Base::Vector;
  using Space   = Base::Space;

  using DirIterator = PreCellDirectionIterator<dim, Integer>;
  using PreCellularGridSpace = Self;

  // static constants
  static constexpr const DGtal::Dimension dimension = Base::dimension;
  static constexpr const DGtal::Dimension DIM = Base::DIM;
  static constexpr const Sign POS = Base::POS;
  static constexpr const Sign NEG = Base::NEG;

  // Neighborhoods, Incident cells, Faces and Cofaces
  using Cells  = Base::template AnyCellCollection<Cell>;
  using SCells = Base::template AnyCellCollection<SCell>;

  // Sets, Maps
  using CellSet   = std::unordered_set<Cell>;
  using SCellSet  = std::unordered_set<SCell>;
  using SurfelSet = std::unordered_set<SCell>;

  template <typename Value> struct CellMap {
    typedef std::unordered_map<Cell, Value> Type;
  };

  template <typename Value> struct SCellMap {
    typedef std::unordered_map<SCell, Value> Type;
  };

  template <typename Value> struct SurfelMap {
    typedef std::unordered_map<SCell, Value> Type;
  };

  // Gave up on line 450
}

} // namespace;