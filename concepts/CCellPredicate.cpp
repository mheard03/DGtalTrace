#pragma once
#include <concepts>
#include <DGtal/base/Common.h>
#include <utility>
#include "CCell.cpp"
#include "DGtalMiscConcepts.cpp"

namespace DGtalTrace {
namespace concepts {

template <typename TFunc>
concept CCellPredicate = requires {
  typename TFunc::Cell;
  requires CXXCell<typename TFunc::Cell>;
  requires CUnaryFunctor<TFunc, typename TFunc::Cell, bool>;
};

} // namespace concepts
} // namespace DGtalTrace
