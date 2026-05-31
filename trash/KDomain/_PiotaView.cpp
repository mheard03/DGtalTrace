#pragma once
#include <DGtal/base/Common.h>
#include <bit>
#include <concepts>
#include <iterator>
#include <ranges>
#include <DGtal/kernel/PointVector.h>
#include "../concepts/BaseConcepts.cpp"
#include "BiotaView.cpp"

namespace DGtalTrace {

namespace {
template <std::integral T, DGtal::Dimension dim>
requires requires { requires dim > 1; }
struct LegalParitiesStruct {
  using type = std::pair<typename BiotaView<T>::Parity, LegalParitiesStruct<T, dim-1>>;
};

template <std::integral T>
struct LegalParitiesStruct<T, 1> {
  using type = std::vector<BiotaView<int>::Parity>;
};
} // anonymous namespace

template <std::integral T, DGtal::Dimension dim>
using LegalParities = typename LegalParitiesStruct<T, dim>::type;

// Primary template (The Recursive Step)
template <typename T, int N>
struct BuildType {
    using type = typename BuildType<T, N - 1>::type;
};

// Explicit Specialization (The Base Case)
template <typename T>
struct BuildType<T, 0> {
    using type = T;
};

template <DGtal::Dimension dim>
std::vector<BiotaView<int>::Parity> getParities(const DGtal::Dimension cellDim) {

}
}