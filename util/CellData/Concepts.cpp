#pragma once
#include <DGtal/base/Common.h>
#include <DGtal/helpers/StdDefs.h>
#include <DGtal/kernel/PointVector.h>
#include <DGtal/topology/KhalimskySpaceND.h>
#include <DGtal/base/CSTLAssociativeContainer.h>
#include <concepts>
#include <type_traits>
#include "../concepts/DGtalConcepts.cpp"
#include "../concepts/CCell.cpp"

namespace DGtalTrace {
  namespace concepts {

    template <typename T, int dim = -1> 
    concept CCellToDataContainer = requires {
      typename T::key_type;
      typename T::mapped_type;
      typename T::value_type;
      std::same_as<const typename T::key_type, typename T::value_type>;
    } && CXCell<typename T::key_type> && CHasDimension<typename T::key_type>;

    template <typename T, int dim = -1> 
    concept CPointToDataContainer = requires {
      typename T::key_type;
      typename T::mapped_type;
      typename T::value_type;
      std::same_as<const typename T::key_type, typename T::value_type>;
      CPoint<typename T::key_type>;
    } && CHasDimension<typename T::key_type>;

    template <typename T> 
    concept CIntToDataContainer = std::ranges::random_access_range<T> && requires {
      typename T::iterator;
      typename T::size_type;
    } && std::random_access_iterator<typename T::iterator> && requires {
      std::is_integral_v<typename T::size_type> == true;
    };

    template <typename T>
    concept CPointToCellFunctor = requires { 
      CPointFunctor<T>;
      CXXCell<typename T::Value>;
    };

    template <typename T>
    concept CCellKeyMapper = requires { 
      CPointFunctor<T>;
      CXXCell<typename T::Value>;
    };
  }
}
