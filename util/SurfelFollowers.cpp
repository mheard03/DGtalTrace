#pragma once
#include <DGtal/base/BasicFunctors.h>
#include <DGtal/base/ConstAlias.h>
#include <DGtal/helpers/StdDefs.h>
#include <optional>
#include <ranges>
#include <DGtal/topology/SurfelAdjacency.h>
#include <DGtal/topology/SurfelNeighborhood.h>
#include "../concepts/CXCellularGridSpaceND.cpp"
#include "../concepts/dgtal/CPredicate.cpp"
#include "../concepts/dgtal/CUnaryFunctor.cpp"
#include "../concepts/dgtal/CSurfelPredicate.cpp"
#include "Globals.cpp"
#include "ConstSurfelPredicate.cpp"
#include <DGtal/base/Common.h>
#include <DGtal/base/FunctorHolder.h>
#include <utility>

namespace DGtalTrace {

namespace {

template <concepts::CXCellularGridSpaceND TKSpace>
struct SurfelFollowersCore {
  typedef TKSpace KSpace;
  typedef KSpace::Surfel Surfel;
  typedef std::vector<Surfel> SurfelVector;
  typedef DGtalTrace::functors::TrueSurfelPredicate<TKSpace> DefaultFilter;
  static inline const DefaultFilter defaultFilter;

private:
  typedef KSpace::PreCellularGridSpace KPreSpace;
  typedef KPreSpace::Surfel PreSurfel;
  typedef std::array<PreSurfel, 3> PreSurfelArray;

  static PreSurfelArray getPreSurfelView(const PreSurfel& aSurfel, const bool fwd, const bool int2ext) {
    const KPreSpace* myKPreSpace = &detail::preSpaces<KPreSpace>;
    const DGtal::SurfelAdjacency<KPreSpace::dimension> sa(int2ext);
    DGtal::SurfelNeighborhood<KPreSpace> sn;
    sn.init(myKPreSpace, &sa, aSurfel);
    const DGtal::Dimension track_dir = *(myKPreSpace->sDirs(aSurfel));
    bool pos = myKPreSpace->sDirect(aSurfel, track_dir);
    if (!fwd) pos = !pos;

    PreSurfelArray result;
    if (int2ext) {
      result[0] = sn.follower1(track_dir, pos);
      result[1] = sn.follower2(track_dir, pos);
      result[2] = sn.follower3(track_dir, pos);
    }
    else {
      result[2] = sn.follower1(track_dir, pos);
      result[1] = sn.follower2(track_dir, pos);
      result[0] = sn.follower3(track_dir, pos);
    }
    return result;
  }

private:
  template <concepts::CSurfelPredicate TFilter = DefaultFilter>
  static inline auto getSurfelView(const PreSurfel& aSurfel, const bool fwd, const bool int2ext, const TFilter& aFilter = defaultFilter)
  requires concepts::CPreCellularGridSpaceND<KSpace> {
    return getPreSurfelView(aSurfel, fwd, int2ext) | std::views::filter(aFilter);
  }

  template <concepts::CSurfelPredicate TFilter = DefaultFilter>
  static inline auto getSurfelView(const PreSurfel& aSurfel, const bool fwd, const bool int2ext, const KSpace* aKSpace, const TFilter& aFilter = defaultFilter) {
    if constexpr (concepts::CPreCellularGridSpaceND<KSpace>) {
      return getSurfelView(aSurfel, fwd, int2ext, aFilter);
    }
    else if constexpr (concepts::CCellularGridSpaceND<KSpace>) {
      return getPreSurfelView(aSurfel, fwd, int2ext) | 
              std::views::filter([aKSpace](auto& s) { return aKSpace->sIsValid(s); }) | 
              std::views::transform([aKSpace](auto& s) { return aKSpace->sCell(s); }) | 
              std::views::filter(aFilter);
    }
  }

public:
  template <concepts::CSurfelPredicate TFilter = DefaultFilter>
  static void allOutbound(SurfelVector& target, const Surfel& aSurfel, const bool int2ext, const KSpace* aKSpace, const TFilter& aFilter = *defaultFilter) {
    fillVector(target, getSurfelView(aSurfel, true, int2ext, aKSpace, aFilter));
  }

  template <concepts::CSurfelPredicate TFilter = DefaultFilter>
  static void allInbound(SurfelVector& target, const Surfel& aSurfel, const bool int2ext, const KSpace* aKSpace, const TFilter& aFilter = *defaultFilter) {
    fillVector(target, getSurfelView(aSurfel, false, int2ext, aKSpace, aFilter));
  }

  template <concepts::CSurfelPredicate TFilter = DefaultFilter>
  static bool next(Surfel& aSurfel, const bool int2ext, const KSpace* aKSpace, const TFilter& aFilter = *defaultFilter) {
    auto candidates = getSurfelView(aSurfel, true, int2ext, aKSpace, aFilter);
    if (std::ranges::empty(candidates)) return false;
    aSurfel = *std::ranges::begin(candidates);
    return true;
  }

  template <concepts::CSurfelPredicate TFilter = DefaultFilter>
  static bool prev(Surfel& aSurfel, const bool int2ext, const KSpace* aKSpace, const TFilter& aFilter = *defaultFilter) {
    // I'm taking this on faith, since this is the default/intended DGtal behavior
    auto candidates = getSurfelView(aSurfel, false, int2ext, aKSpace, aFilter);
    if (std::ranges::empty(candidates)) return false;
    aSurfel = *std::ranges::begin(candidates);
    return true;
  }

  template <concepts::CSurfelPredicate TFilter = DefaultFilter>
  static bool nextUnique(Surfel& aSurfel, const KSpace* aKSpace, const TFilter& aFilter = *defaultFilter) {
    SurfelVector candidates;
    allOutbound(candidates, aSurfel, true, aKSpace, aFilter);
    if (candidates.size() != 1) return false;
    
    aSurfel = candidates[0];
    return true;
  }

  template <concepts::CSurfelPredicate TFilter = DefaultFilter>
  static bool prevUnique(Surfel& aSurfel, const KSpace* aKSpace, const TFilter& aFilter = *defaultFilter) {
    Surfel target = aSurfel;
    SurfelVector candidates;
    allInbound(candidates, aSurfel, true, aKSpace, aFilter);
    if (candidates.size() != 1) return false;
    
    Surfel candidateNext = candidates[0];
    if (!nextUnique(candidateNext, aKSpace, aFilter)) return false;
    if (candidateNext != target) return false;

    aSurfel = candidates[0];
    return true;
  }

private:
  static inline void fillVector(SurfelVector& target, auto&& range) {
    target.clear();
    target.insert(target.begin(), range.begin(), range.end());
    target.shrink_to_fit();
  }

  static inline SurfelVector makeVector(auto&& range) {
    SurfelVector target;
    fillVector(target, range);
    return target;
  }

};

}


#define SURFEL_FOLLOWERS_BODY(fnStatic, fnConst) \
  typedef SurfelFollowersCore<TKSpace> Base; \
  typedef Base::DefaultFilter DefaultFilter; \
  static inline const DefaultFilter* defaultFilter = &Base::defaultFilter; \
\
public: \
  typedef Base::KSpace KSpace; \
  typedef Base::Surfel Surfel; \
  typedef Base::SurfelVector SurfelVector; \
  \
  template <concepts::CSurfelPredicate TFilter = DefaultFilter> \
  fnStatic void allOutbound(SurfelVector& target, const Surfel& aSurfel, const bool int2ext, const TFilter& aFilter = *defaultFilter) fnConst { \
    return Base::allOutbound(target, aSurfel, int2ext, myKPtr, aFilter); \
  } \
  \
  template <concepts::CSurfelPredicate TFilter = DefaultFilter> \
  fnStatic void allInbound(SurfelVector& target, const Surfel& aSurfel, const bool int2ext, const TFilter& aFilter = *defaultFilter) fnConst { \
    return Base::allInbound(target, aSurfel, int2ext, myKPtr, aFilter); \
  } \
  \
  template <concepts::CSurfelPredicate TFilter = DefaultFilter> \
  fnStatic bool next(Surfel& aSurfel, const bool int2ext, const TFilter& aFilter = *defaultFilter) fnConst { \
    return Base::next(aSurfel, int2ext, myKPtr, aFilter); \
  } \
  \
  template <concepts::CSurfelPredicate TFilter = DefaultFilter> \
  fnStatic bool nextUnique(Surfel& aSurfel, const TFilter& aFilter = *defaultFilter) fnConst { \
    return Base::nextUnique(aSurfel, myKPtr, aFilter); \
  } \
  \
  template <concepts::CSurfelPredicate TFilter = DefaultFilter> \
  fnStatic bool prev(Surfel& aSurfel, const bool int2ext, const TFilter& aFilter = *defaultFilter) fnConst { \
    return Base::prev(aSurfel, int2ext, myKPtr, aFilter); \
  } \
  \
  template <concepts::CSurfelPredicate TFilter = DefaultFilter> \
  fnStatic bool prevUnique(Surfel& aSurfel, const TFilter& aFilter = *defaultFilter) fnConst { \
    return Base::prevUnique(aSurfel, myKPtr, aFilter); \
  }


template <concepts::CXCellularGridSpaceND TKSpace>
class SurfelFollowers;

template <concepts::CCellularGridSpaceND TKSpace>
class SurfelFollowers<TKSpace> {
  SURFEL_FOLLOWERS_BODY(,const)

  SurfelFollowers(DGtal::ConstAlias<KSpace> ks) : myKPtr(&ks) {}

private:
  const KSpace* myKPtr;
};

template <concepts::CPreCellularGridSpaceND TKSpace>
class SurfelFollowers<TKSpace> {
  SURFEL_FOLLOWERS_BODY(static,)

private:
  static inline const KSpace* myKPtr = &detail::preSpaces<KSpace>;
};
}