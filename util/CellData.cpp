#pragma once

#include <DGtal/kernel/domains/HyperRectDomain.h>
#include <boost/concept/assert.hpp>
#include <DGtal/base/Common.h>
#include <DGtal/helpers/StdDefs.h>
#include <DGtal/topology/KhalimskySpaceND.h>
#include <DGtal/base/CSTLAssociativeContainer.h>
#include <cstdint>
#include <sys/types.h>
#include "concepts/CCell.cpp"
#include "kernel/KhalimskyDomainND.cpp"

namespace DGtalTrace {

typedef DGtal::Z2i::KSpace::Cell TCell;
//template <concepts::CXXCell TCell, typename T>
template <typename T, >
class CellData {
 public:
  typedef TCell Cell;
  typedef typename Cell::CellularGridSpace KSpace;
  typedef typename KSpace::Point Point;
  typedef typename KSpace::Vector Vector;
  typedef KhalimskyDomainND<KSpace> KDomain;
  typedef DGtal::HyperRectDomain<KSpace::Space> Domain;
  static constexpr const DGtal::Dimension dimension = KSpace::dimension;

 public:
  typedef Cell key_type;
  typedef T value_type;
  typedef KSpace::Size size_type;
  
 public:
  const T defaultValue;
 private:

    typedef std::vector<T> data;
  const KDomain kCoordsDomain;
  const Domain coordsDomain;


};
} // namespace DGtalTrace




/*
// static const uint16_t stride = std::numeric_limits<uint16_t>::max() / 4;
// const StripeMap stripes;

template <class TContainer>
class CellData {
  BOOST_CONCEPT_ASSERT(( DGtal::concepts::CSTLAssociativeContainer<TContainer> ));
  static constexpr bool isVectorStorage = std::is_integral_v<typename TContainer::key_type>;

  typedef typename TContainer::key_type key_type;
  typedef typename TContainer::mapped_type mapped_type;
  typedef typename TContainer::value_type value_type;

};
*/