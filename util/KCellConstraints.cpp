#pragma once
#include <type_traits>
#include <DGtal/base/Common.h>
#include <DGtal/helpers/StdDefs.h>

template <typename T>
concept CAnyCell = requires {
      typename T::CellularGridSpace;
      typename T::PreCellularGridSpace;
} && (
    std::same_as<T, typename T::CellularGridSpace::Cell>    ||
    std::same_as<T, typename T::CellularGridSpace::SCell>   ||
    std::same_as<T, typename T::PreCellularGridSpace::Cell> ||
    std::same_as<T, typename T::PreCellularGridSpace::SCell>
);

template <typename T>
concept CPreCell = CAnyCell<T> && (
    std::same_as<T, typename T::PreCellularGridSpace::Cell> ||
    std::same_as<T, typename T::PreCellularGridSpace::SCell>
);

template <typename T>
concept CCell = CAnyCell<T> && !CPreCell<T>;

template <typename T>
concept CSignedCell = CAnyCell<T> && (
    std::same_as<T, typename T::CellularGridSpace::SCell>   ||
    std::same_as<T, typename T::PreCellularGridSpace::SCell>
);

template <typename T>
concept CSUnsignedCell = CAnyCell<T> && !CSignedCell<T>;

/*
template <class T>
struct cell_traits {};

template <CSignedCell C>
struct cell_traits<C> {
  typedef typename C::CellularGridSpace KSpace;
  typ std::is_same<T, typename T::PreCellularGridSpace::Cell>,
  std::is_same<T, typename T::PreCellularGridSpace::SCell>
  typedef typename C::CellularGridSpace KSpace;
};

template <CSUnsignedCell C>
struct cell_traits<C> {
  typedef typename C::PreCellularGridSpace KSpace;
};
*/
/*
template <typename T, typename = void>
struct is_cell : std::false_type {};

template <typename T>
struct is_cell<T, std::void_t<typename T::CellularGridSpace, typename T::PreCellularGridSpace>> : std::disjunction<
  std::is_same<T, typename T::CellularGridSpace::Cell>,
  std::is_same<T, typename T::CellularGridSpace::SCell>,
  std::is_same<T, typename T::PreCellularGridSpace::Cell>,
  std::is_same<T, typename T::PreCellularGridSpace::SCell>
> {};

template <typename T, typename = void>
struct is_precell : std::false_type {};

template <typename T>
struct is_precell<T, std::void_t<typename T::CellularGridSpace, typename T::PreCellularGridSpace>> : std::disjunction<
  std::is_same<T, typename T::PreCellularGridSpace::Cell>,
  std::is_same<T, typename T::PreCellularGridSpace::SCell>
> {};

template <typename T, typename = void>
struct is_scell : std::false_type {};

template <typename T>
struct is_scell<T, std::void_t<typename T::CellularGridSpace, typename T::PreCellularGridSpace>> : std::disjunction<
  std::is_same<T, typename T::CellularGridSpace::SCell>,
  std::is_same<T, typename T::PreCellularGridSpace::SCell>
> {};


template<class T> constexpr bool is_cell_v = is_cell<T>::value;
template<class T> constexpr bool is_precell_v = is_precell<T>::value;
template<class T> constexpr bool is_scell_v = is_scell<T>::value;
*/