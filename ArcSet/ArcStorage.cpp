#pragma once
#include "../concepts/CCell.cpp"
#include "../util/CellTypeTraits.cpp"
#include "../util/KCoordsAdjuster.cpp"
#include <DGtal/helpers/StdDefs.h>
#include <DGtal/kernel/NumberTraits.h>
#include <DGtal/kernel/PointVector.h>
#include <cstdint>
#include <ranges>
#include <vector>
#include <type_traits>

template <typename Integer = uint16_t, typename CacheAddress = uint32_t>
class CharTable {

  struct FCMeta {
    typedef DGtal::PointVector<2, Integer> Point;
    Point startKCoords;
    Point endKCoords;
    uint32_t storageBlobOffset;
  };

  std::string storageBlob_;  
  std::vector<CacheAddress> offsets_;

public:
  static inline constexpr DGtal::Dimension dimension = 2;

  template <std::ranges::input_range Rows>
  CharTable() {
    offsets_.push_back(0);
    std::size_t total = 0;
    for (auto &&row : rows)
      total += std::ranges::size(row);
    storageBlob_.reserve(total);

    for (auto &&row : rows) {
      storageBlob_.append(std::ranges::begin(row), std::ranges::end(row));
      offsets_.push_back(static_cast<CacheAddress>(storageBlob_.size()));
    }
  }

  std::size_t num_rows() const noexcept { return offsets_.size() - 1; }

  std::string_view operator[](std::size_t row) const {
    return {storageBlob_.data() + offsets_[row], offsets_[row + 1] - offsets_[row]};
  }

  auto rows() const {
    return std::views::iota(std::size_t{0}, num_rows()) |
           std::views::transform([this](std::size_t i) { return (*this)[i]; });
  }
};