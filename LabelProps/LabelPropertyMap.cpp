#pragma once
#include <DGtal/base/CConstSinglePassRange.h>
#include <boost/iostreams/detail/streambuf/indirect_streambuf.hpp>
#include <cinttypes>
#include <cstdint>
#include <ranges>
#include <functional>
#include <optional>
#include <set>
#include <unordered_map>
#include <type_traits>
#include <DGtal/base/Common.h>
#include <DGtal/io/Color.h>
#include <utility>
#include "../concepts/dgtal/images/CConstImage.cpp"
#include "../concepts/dgtal/CLabel.cpp"
#include "../concepts/dgtal/ranges/CConstSinglePassRange.cpp"
#include "../concepts/CIsOptional.cpp"
#include "../util/OptValue.cpp"

// requires std::assignable_from<TValue&, int>

template <>
struct std::hash<DGtal::Color> {
  auto operator()(const DGtal::Color color) const {
    return hasher(color.getRGBA());
  }
private:
  static inline const std::hash<DGtal::uint32_t> hasher;
};


namespace DGtalTrace {

enum class ContrastLevels {
  NONE = 0,
  PARTIAL,
  FULL
};



namespace {

/*
struct OptionalHash {
  
  template <typename T>
  std::size_t operator()(const std::optional<T>& opt) const {
    return (opt.has_value()) ? std::hash<T>{}(*opt) : 0;
  }
  
  template <concepts::CIsOptional T>
  static inline constexpr std::hash<detail::UnwrapOptValue<T>> hasher;
  hasher

  template <concepts::CIsOptional T>
  std::size_t operator()(const T& opt) const {
    return (opt.has_value()) ? hasher(*opt) : 0;
  }
};
*/
}

template <concepts::CConstImage TImage, typename TValue, bool bIsMultiValue>
requires (!bIsMultiValue || std::three_way_comparable<TValue>)  // three-way comparable enables AssociativeContainer<TValue>
class LabelPropertyMap {
public:
  typedef std::unordered_map<detail::OptValue<typename TImage::Value>, 
                             std::conditional_t<bIsMultiValue, std::set<TValue>, TValue>> Map;
  typedef Map::key_type key_type;
  typedef Map::mapped_type mapped_type;
  typedef TValue mapped_inner_type;
  static inline const bool IsMultiValue = bIsMultiValue;

  LabelPropertyMap() {}
  LabelPropertyMap(const Map& other) : myData(other) {}
  LabelPropertyMap(const Map&& other) : myData(other) {}

  inline mapped_type& operator[](const key_type& key) {
    return myData[key];
  }
  inline const mapped_type& operator[](const key_type& key) const {
    return myData.at(key);
  }  
  inline mapped_type& operator[](key_type&& key) {
    return myData[std::move(key)];
  }

  bool hasContrast(const key_type& key1, const key_type& key2) {
    if (key1 == key2) return false;
    return myData.at(key1) != myData.at(key2);
  }

  ContrastLevels contrastLevel(const key_type& key1, const key_type& key2) {
    if (key1 == key2) return ContrastLevels::NONE;
    if constexpr (!IsMultiValue) {
      return (myData.at(key1) == myData.at(key2)) ? ContrastLevels::NONE : ContrastLevels::FULL;
    }
    else {
      const mapped_type& set1 = myData.at(key1);
      const mapped_type& set2 = myData.at(key2);
      if (set1.empty() && set2.empty()) return ContrastLevels::NONE;  // two empty sets
      if (set1.empty() || set2.empty()) return ContrastLevels::FULL;  // one empty set
      
      const size_t set1Size = set1.size();
      if (set1Size != set2.size()) {
        return (set1 == set2) ? ContrastLevels::NONE : ContrastLevels::PARTIAL;
      }

      // set1 and set2 have the same non-zero length
      std::size_t matchCount = 0;
      std::size_t missCount = 0;
      auto a = set1.begin(), aEnd = set1.end();
      auto b = set2.begin(), bEnd = set2.end();
      while (a != aEnd && b != bEnd) {
        if (*a < *b) { ++missCount; ++a; }
        else if (*b < *a) { ++missCount; ++b; }
        else { ++matchCount; ++a; ++b; }
        if (matchCount > 0 && missCount > 0) return ContrastLevels::PARTIAL;
      }
      if (matchCount == set1Size) return ContrastLevels::NONE;
      if (missCount == set1Size) return ContrastLevels::FULL;
      throw "wtaf";
    }
  }

  Map& data() {
    return myData;
  }
  const Map& data() const {
    return myData;
  }

private:
  Map myData;
};


} // end namespace DGtalTrace

/*

template <concepts::CConstImage TImage, typename TValue>
requires DGtal::concepts::CConstSinglePassRange<TValue>
class LabelPropertyMap : public LabelPropertyMapBase<LabelPropertyMap<TImage, TValue>> {
 public:
  typedef std::unordered_map<detail::OptValue<typename TImage::Value>, TValue, OptionalHash> Map;
  typedef Map::key_type key_type;
  typedef Map::mapped_type mapped_type;

  using LabelPropertyMap::LabelPropertyMap;

  ContrastLevels contrastLevel(const key_type& key1, const key_type& key2) const
  requires concepts::CIsTrue<is_specialization<mapped_type, std::set>> || concepts::CIsTrue<is_specialization<mapped_type, std::unordered_set>> {
    const mapped_type value1 = (*this)[key1];
    const mapped_type value2 = (*this)[key2];
    std::set<int> s;
    s.
    if (value1)
  }
};

} // end namespace DGtalTrace

enum class BorderType {
  NONE = 0,
  POLITICAL_SOFT,
  POLITICAL_HARD,
  GEOGRAPHIC
};
*/