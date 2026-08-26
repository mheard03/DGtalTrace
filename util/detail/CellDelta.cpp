#pragma once
#include "../traceHelper.cpp"
#include "../../concepts/CXCellularGridSpaceND.cpp"
#include <DGtal/base/Assert.h>
#include <DGtal/helpers/StdDefs.h>
#include <DGtal/kernel/PointVector.h>
#include <concepts>
#include <type_traits>

namespace DGtalTrace {
namespace detail {

template <DGtal::Dimension DIM, typename TInteger>
struct CellDelta {
  typedef TInteger Integer;
  typedef typename DGtal::PointVector<DIM, TInteger> Vector;

  DGtal::Dimension dim;
  Integer delta;

  inline Integer abs() const {
    return std::abs(delta);
  }
  auto opp() const {
    return CellDelta<DIM, TInteger>{dim, -delta};
  }
  auto relative(TInteger relativeDelta) const {
    TInteger newDelta = (delta > 0) ? relativeDelta : -relativeDelta;
    auto result = CellDelta<DIM, TInteger>{dim, relativeDelta};
    ASSERT(result.isValid());
    return result;
  }
  bool isValid() const {
    return dim < DIM && delta != 0 && abs() <= 2;
  }
  operator Vector() const {
    ASSERT(isValid());
    Vector v = Vector::ZERO;
    v[dim] = delta;
    return v;
  }

  void selfDisplay(std::ostream &out) const {
    out << "[Delta " << delta << " on dim " << dim << "]";
  }
};

} // end namespace detail

template <concepts::CXCellularGridSpace2D TKSpace>
using CellDeltaSelector = detail::CellDelta<TKSpace::DIM, typename TKSpace::Integer>;


template <DGtal::Dimension DIM, typename TInteger>
std::ostream& operator<<(std::ostream &out, const DGtalTrace::detail::CellDelta<DIM, TInteger> &object) {
  object.selfDisplay(out);
  return out;
}

} // end namespace DGtalTrace