#pragma once
// #include <DGtal/base/Common.h>
// #include <DGtal/helpers/StdDefs.h>
// #include <type_traits>
#include "../concepts/CCell.cpp"

namespace DGtalTrace {

template <concepts::CXXCell TCell>
struct CellTypeTraits {
  using USelf = typename TCell::CellularGridSpace::Cell;
  using SSelf = typename TCell::CellularGridSpace::SCell;  
  using Self =  std::conditional_t<concepts::CSXCell<TCell>, SSelf, USelf>;  

  using KPreSpace = TCell::PreCellularGridSpace;
  using UPreSelf =  typename KPreSpace::Cell;
  using SPreSelf =  typename KPreSpace::SCell;
  using PreSelf =   std::conditional_t<concepts::CSXCell<TCell>, SPreSelf, UPreSelf>;  
};

}