#pragma once
// #include <DGtal/base/Common.h>
// #include <DGtal/helpers/StdDefs.h>
// #include <type_traits>
#include "../concepts/CCell.cpp"

namespace DGtalTrace {

template <concepts::CXXCell TCell>
struct CellTypeTraits {
 private:
  static constexpr bool IsSigned = concepts::CSXCell<TCell>;
 public:
  using USelf =    TCell::CellularGridSpace::Cell;
  using SSelf =    TCell::CellularGridSpace::SCell;  
  using UPreSelf = TCell::PreCellularGridSpace::Cell;
  using SPreSelf = TCell::PreCellularGridSpace::SCell;
  using PreSelf =  std::conditional_t<concepts::CSXCell<TCell>, SPreSelf, UPreSelf>;
};

}