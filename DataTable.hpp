#pragma once
#include <memory>
#include <string>
#include <map>
#include <concepts>
#include <ranges>
#include <unordered_map>
#include <variant>
#include <vector>
#include <DGtal/base/Common.h>

namespace DGtalTrace {

namespace {
  template <typename T, typename... Ts>
  inline constexpr size_t CountOf = (size_t(std::is_same_v<T, Ts>) + ... + 0);

  template <typename... Ts>
  concept CUniqueTypes = ((CountOf<Ts, Ts...> == 1) && ...);
}


template <typename Table, typename T>
class DataColumn;

template <typename TKey, typename... TValues>
requires CUniqueTypes<TValues...> && requires (TKey lbl, TKey lbl2) {
  { std::hash<TKey>()(lbl) } -> std::convertible_to<size_t>;
  lbl == lbl2;
}
class DataTable {
public:
  using Key = TKey;

  private:
  using Self = DataTable<Key, TValues...>;
  using KeyIndex = std::unordered_map<Key, size_t>;

  template <typename T>
  static inline constexpr bool HasT = (std::is_same_v<T, TValues> || ...);  

  // DataColumn
  template <typename T> requires HasT<T>
  class DataColumn {
    using Table = DataTable<Key, TValues...>;
    using Value = T;
    friend Table;

  public:
    DataColumn(std::shared_ptr<const KeyIndex> aKeyIndex, const T& aDefaultValue) : keyIndex(std::move(aKeyIndex)), defaultValue(aDefaultValue) {}

    const T& at(const Key& key) const {
      return values.at(keyIndex->at(key));
    }

  private:
    T& atIndex(const size_t index) {
      return values[index];
    }

    void resize(const size_t newSize) {
      values.resize(newSize, defaultValue);
    };

    T defaultValue = T();
    std::vector<T> values;
    std::shared_ptr<const KeyIndex> keyIndex;
  };

public:
  using Column = std::variant<DataColumn<TValues>...>;

  DataTable() = default;

  // Copies get their own key index, and their columns are rebound to it
  DataTable(const DataTable& other)
    : keyIndices(std::make_shared<KeyIndex>(*other.keyIndices)), columns(other.columns) {
    rebindColumns();
  }

  DataTable& operator=(const DataTable& other) {
    if (this != &other) {
      *this = DataTable(other);
    }
    return *this;
  }

  // Moves carry the key index along, so columns stay bound to it
  DataTable(DataTable&&) = default;
  DataTable& operator=(DataTable&&) = default;

  // addColumn
  template <typename T> requires HasT<T>
  void addColumn(const std::string& colName, const T& defaultValue = T()) {
    DataColumn<T> col(keyIndices, defaultValue);
    col.resize(size());
    columns.insert_or_assign(colName, col);
  }

  // getColumn
  template <typename T> requires HasT<T>
  DataColumn<T>& getColumn(const std::string& colName) {
    return std::get<DataColumn<T>>(columns.at(colName));
  }

  Column& getColumn(const std::string& colName) {
    return columns.at(colName);
  }

  // const getColumn
  template <typename T> requires HasT<T>
  const DataColumn<T>& getColumn(const std::string& colName) const {
    return std::get<DataColumn<T>>(columns.at(colName));
  }

  const Column& getColumn(const std::string& colName) const {
    return columns.at(colName);
  }

  // insert_or_assign
  template <typename T> requires HasT<T>
  void insert_or_assign(const std::string& colName, const TKey& key, const T& value) {
    auto& col = getColumn<T>(colName);
    col.atIndex(indexOf(key)) = value;
  }

  inline void insert_or_assign(const std::string& colName, const TKey& key, const char* value)
  requires (!HasT<char*>) {
    insert_or_assign<std::string>(colName, key, value);
  }

  // char[N]

  /*
  void insert_or_assign(const std::string& colName, const TKey& key, const std::variant<TValues...>& value) {
    auto& col = getColumn(colName);
    std::visit([&](auto& c) { c.atIndex(indexOf(key)) = value; }, col);    
  }
  */

  // size
  size_t size() const {
    return keyIndices->size();
  }

  // selfDisplay
  void selfDisplay(std::ostream& out) const {
    out << "[DataTable() " << columns.size() << "x" << keyIndices->size() << ")]" << std::endl;

    // header
    out << "KEY";
    for (auto colName : std::views::keys(columns)) {
      out << ", " << colName;
    }
    out << std::endl;

    // 10 rows
    for (auto [key, index] : *keyIndices | std::ranges::views::take(10)) {
      out << key;
      for (auto col : std::views::values(columns)) {
        std::visit([&](auto& c) { out << ", " << c.at(key); }, col);
      }
      out << std::endl;
    }

    // 11th row
    if (keyIndices->size() > 10) {
      out << (keyIndices->size() - 10) << " more..." << std::endl;
    }
  }

private:
  void rebindColumns() {
    for (auto& [name, col] : columns) {
      std::visit([&](auto& c) { c.keyIndex = keyIndices; }, col);
    }
  }

  size_t indexOf(const TKey& key) {
    auto [kvp, wasInserted] = keyIndices->try_emplace(key, size());
    size_t index = kvp->second;
    if (wasInserted) {
      for (auto& [key, col] : columns) {
        std::visit([&](auto& c) { c.resize(index + 1); }, col);
      }
    }
    return index;
  }

  std::shared_ptr<KeyIndex> keyIndices = std::make_shared<KeyIndex>();
  std::map<std::string, Column> columns;
};

template <typename TKey, typename... TValues>
std::ostream& operator<<(std::ostream & out, const DataTable<TKey, TValues...> & object) {
  object.selfDisplay(out);
  return out;
}

} // end namespace


/*
        auto try_type = [&]<typename T>(std::type_identity<T>) {
            if (!result && type_name<T> == name)
                result = DataColumn<T>{};
        };

        // 2. The fold calls try_type once per type in TValues:
        //    try_type(std::type_identity<int>{}),
        //    try_type(std::type_identity<double>{}),
        //    try_type(std::type_identity<std::string>{});
        (try_type(std::type_identity<TValues>{}), ...);
        */