#pragma once
#include <DGtal/topology/KhalimskySpaceND.h>
#include "../concepts/CXCellularGridSpaceND.cpp"

namespace DGtalTrace {
namespace detail {

template <concepts::CPreCellularGridSpaceND KSpace>
constexpr KSpace preSpaces;

template <concepts::CCellularGridSpaceND KSpace>
KSpace infiniteKSpaces;

} // end namespace detail
} // end namespace DGtalTrace