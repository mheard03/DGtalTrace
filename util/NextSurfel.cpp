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
struct SurfelFollowersImpl {
  typedef TKSpace KSpace;
  typedef KSpace::Surfel Surfel;
  typedef DGtalTrace::functors::TrueSurfelPredicate<TKSpace> DefaultFilter;
  static inline const DefaultFilter defaultFilter;

private:
  typedef KSpace::PreCellularGridSpace KPreSpace;
  typedef KPreSpace::Surfel PreSurfel;
  typedef std::array<PreSurfel, 3> PreSurfelArray;

  static PreSurfelArray getPreSurfels(const PreSurfel& aSurfel, const bool int2ext) {
    const KPreSpace* myKPreSpace = &detail::preSpaces<KPreSpace>;
    const DGtal::SurfelAdjacency<KPreSpace::dimension> sa(int2ext);
    DGtal::SurfelNeighborhood<KPreSpace> sn;
    sn.init(myKPreSpace, &sa, aSurfel);
    const DGtal::Dimension track_dir = *(myKPreSpace->sDirs(aSurfel));
    bool pos = myKPreSpace->sDirect(aSurfel, track_dir);

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

public:
  template <concepts::CSurfelPredicate TFilter = DefaultFilter>
  static inline auto getSurfels(const PreSurfel& aSurfel, const bool int2ext, const TFilter& aFilter = defaultFilter)
  requires concepts::CPreCellularGridSpaceND<KSpace> {
    return getPreSurfels(aSurfel, int2ext) | std::views::filter(aFilter);
  }

  template <concepts::CSurfelPredicate TFilter = DefaultFilter>
  static inline auto getSurfels(const PreSurfel& aSurfel, const bool int2ext, const KSpace* aKSpace, const TFilter& aFilter = defaultFilter) {
    if constexpr (concepts::CPreCellularGridSpaceND<KSpace>) {
      return getSurfels(aSurfel, int2ext, aFilter);
    }
    else if constexpr (concepts::CCellularGridSpaceND<KSpace>) {
      return getPreSurfels(aSurfel, int2ext) | 
              std::views::filter(aKSpace->sIsValid) | 
              std::views::transform(aKSpace->sCell) | 
              std::views::filter(aFilter);
    }
  }
};

}

template <concepts::CXCellularGridSpaceND TKSpace>
class SurfelFollowers {
  typedef SurfelFollowersImpl<TKSpace> Logic;
  typedef Logic::DefaultFilter DefaultFilter;
  static inline const DefaultFilter* defaultFilter = &Logic::defaultFilter;

public:
  typedef TKSpace KSpace;
  typedef KSpace::Surfel Surfel;
  typedef std::vector<Surfel> SurfelVector;

  SurfelFollowers() requires concepts::CPreCellularGridSpaceND<KSpace> : myKSpace(nullptr) {}
  SurfelFollowers(DGtal::ConstAlias<KSpace> ks) : myKSpace(&ks) {}

  template <concepts::CSurfelPredicate TFilter = DefaultFilter>
  void all(SurfelVector& target, const Surfel& aSurfel, const bool int2ext, const TFilter& aFilter = *defaultFilter) const {
    fillVector(target, Logic::getSurfels(aSurfel, int2ext, myKSpace, aFilter));
  }

  template <concepts::CSurfelPredicate TFilter = DefaultFilter>
  SurfelVector all(const Surfel& aSurfel, const bool int2ext, const TFilter& aFilter = *defaultFilter) const {
    return makeVector(Logic::getSurfels(aSurfel, int2ext, myKSpace, aFilter));
  } 

  template <concepts::CSurfelPredicate TFilter = DefaultFilter>
  bool next(Surfel& aSurfel, const bool int2ext, const TFilter& aFilter = *defaultFilter) const {
    auto candidates = Logic::getSurfels(aSurfel, int2ext, myKSpace, aFilter);
    if (std::ranges::empty(candidates)) return false;
    aSurfel = *std::ranges::begin(candidates);
    return true;
  }

  template <concepts::CSurfelPredicate TFilter = DefaultFilter>
  bool nextUnique(Surfel& aSurfel, const bool int2ext, const TFilter& aFilter = *defaultFilter) const {
    SurfelVector candidates = all(aSurfel, int2ext, aFilter);
    if (candidates.size() == 1) {
      aSurfel = candidates[0];
      return true;
    }
    return false;
  }

private:
  const KSpace* myKSpace;

  inline void fillVector(SurfelVector& target, auto&& range) const {
    target.clear();
    target.reserve(3);
    target.insert(target.begin(), range.begin(), range.end());
    target.shrink_to_fit();
  }

  inline SurfelVector makeVector(auto&& range) const {
    SurfelVector target;
    fillVector(target, range);
    return target;
  }
};

}



/*
template <concepts::CXCellularGridSpaceND TKSpace>
struct PreSurfelFollowers {
  typedef TKSpace KSpace;
  typedef KSpace::Surfel Surfel;
  typedef KSpace::PreCellularGridSpace KPreSpace;
  typedef KPreSpace::Surfel PreSurfel;
  typedef std::array<PreSurfel, 3> PreSurfelArray;
  typedef DGtal::functors::Identity Identity;
  typedef PreSurfelRangePromoter<KSpace, PreSurfelArray> PSRP;
  using PSRPOutput = decltype(std::declval<PSRP>().validSurfels(std::declval<PreSurfelArray>()));
  using PSRPFat = decltype(PSRP::filterAndTransform);

  static PreSurfelArray getPreSurfels(const PreSurfel& aSurfel, const bool int2ext) {
    const KPreSpace* myKPreSpace = &detail::preSpaces<KPreSpace>;
    const DGtal::SurfelAdjacency<KPreSpace::dimension> sa(int2ext);
    DGtal::SurfelNeighborhood<KPreSpace> sn;
    sn.init(myKPreSpace, &sa, aSurfel);
    const DGtal::Dimension track_dir = *(myKPreSpace->sDirs(aSurfel));
    bool pos = myKPreSpace->sDirect(aSurfel, track_dir);

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

  template <concepts::CSurfelPredicate TFilter = Identity>
  static auto getSurfels(const PreSurfel& aSurfel, const bool int2ext, const TFilter& = fnIdentity) {
    return getPreSurfels(aSurfel, int2ext) | PSRP::filterAndTransform(myKPreSpace) | std::views::filter(fnIdentity);
  }

  static void getSurfelsEx(std::vector<Surfel>& result, const PreSurfel& aSurfel, const bool int2ext) {
    auto surfels = getSurfels(aSurfel, int2ext);
    result.insert(result.begin(), surfels.begin(), surfels.end());
  }

  

private:
  static inline const KPreSpace* myKPreSpace = &detail::preSpaces<KPreSpace>;
  static inline const Identity fnIdentity;
};

}


/*

template <concepts::CPreCellularGridSpaceND TKSpace, std::ranges::range TInput>
using PSRPOutput = decltype(std::declval<PreSurfelRangePromoter<TKSpace, TInput>>().doit(std::declval<PreSurfelRangePromoter<TKSpace, TInput>::Input>()));

template <concepts::CCellularGridSpaceND TKSpace, std::ranges::range TInput>
struct PreSurfelRangePromoter<TKSpace, TInput>; 


typedef PreSurfelRangePromoter<DGtal::Z2i::KPreSpace, std::array<DGtal::Z2i::KPreSpace::Surfel, 3>> PSRP;
typedef decltype(std::declval<PSRP>().doit(std::declval<PSRP::Input>())) PSRPOutput;


template <concepts::CPreCellularGridSpaceND TKPreSpace>
struct PreSurfelFollowers {
  typedef TKPreSpace KPreSpace;
  typedef typename KPreSpace::Surfel PreSurfel;


  static std::array<PreSurfel, 3> getPreSurfels(const PreSurfel& aSurfel, const bool int2ext) {
    DGtal::SurfelAdjacency<KPreSpace::dimension> sa(int2ext);
    DGtal::SurfelNeighborhood<KPreSpace> sn;
    sn.init(myKPreSpace, &sa, aSurfel);
    DGtal::Dimension track_dir = *(myKPreSpace->sDirs(aSurfel));
    bool pos = myKPreSpace->sDirect(aSurfel, track_dir);

    std::array<PreSurfel, 3> result;
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
  static inline const KPreSpace* myKPreSpace = &detail::preSpaces<KPreSpace>;
};

}


template <concepts::CXCellularGridSpaceND TKSpace>
struct SurfelFollowers;

template <concepts::CPreCellularGridSpaceND TKSpace>
struct SurfelFollowers<TKSpace> {
  typedef TKSpace KSpace;
  typedef typename KSpace::Surfel Surfel;
  typedef std::vector<Surfel> Surfels;

private:
  typedef PreSurfelFollowers<KSpace> Impl;

  auto getValid(const Surfel &aSurfel, const bool int2ext) {
    return Impl::getPreSurfels(aSurfel, int2ext);
  }

private:
  static inline const KSpace* myKSpace = &detail::preSpaces<KSpace>;
}

template <concepts::CCellularGridSpaceND TKSpace>
struct SurfelFollowers<TKSpace>;

  SurfelFollowersImpl() requires concepts::CPreCellularGridSpaceND<KSpace> : myKSpace(myKPreSpace) {}
  SurfelFollowersImpl(DGtal::ConstAlias<KSpace> ks) : myKSpace(&ks) {}

public:
  template <concepts::CSurfelPredicate TPredicate>
  static void get(Surfels& surfels, const Surfel& aSurfel, const bool int2ext, const TPredicate& pred) {
    surfels.clear();
    surfels.reserve(3);
    auto raw = getRaw(aSurfel, int2ext) | std::views::filter(pred);
    surfels.insert(surfels.begin(), raw.begin(), raw.end());
    surfels.shrink_to_fit();
  }

  template <concepts::CSurfelPredicate TPredicate>
  static std::optional<Surfel> unique(const Surfel& aSurfel, const bool int2ext, const TPredicate& pred) {
    std::optional<Surfel> result;
    for (auto surfel : getRaw(aSurfel, int2ext) | std::views::filter(pred)) {
      if (result.has_value()) return std::nullopt;      
      result = surfel;
    }
    return result;
  }

  template <concepts::CSurfelPredicate TPredicate>
  static std::optional<Surfel> first(const Surfel& aSurfel, const bool int2ext, const TPredicate& pred) {
    std::optional<Surfel> result;
    for (auto surfel : getRaw(aSurfel, int2ext) | std::views::filter(pred)) {
      result = surfel;
      break;
    }
    return result;
  }

private:

};

}
/*

namespace { 

template <concepts::CPreCellularGridSpaceND TKPreSpace>
struct SurfelFollowersBase {
  typedef TKPreSpace KPreSpace;
  typedef typename KPreSpace::Surfel Surfel;
  typedef std::array<Surfel, 3> Surfels;

  template <concepts::CCellularGridSpaceND TKSpace>
  friend struct SurfelFollowers;

  static Surfels getRaw(const KPreSpace* ks, const Surfel& aSurfel, const bool int2ext) {
    DGtal::SurfelAdjacency<KPreSpace::dimension> sa(int2ext);
    DGtal::SurfelNeighborhood<KPreSpace> sn;
    sn.init(ks, &sa, aSurfel);

    DGtal::Dimension track_dir = *(ks->sDirs(aSurfel));
    bool pos = ks->sDirect(aSurfel, track_dir);

    Surfels result;
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


};

} // end anonymous namespace;

template <concepts::CXCellularGridSpaceND TKSpace>
struct SurfelFollowers {
  typedef TKSpace KSpace;
  typedef typename KSpace::Surfel Surfel;
  typedef std::vector<Surfel> Surfels;

 private:
  typedef typename KSpace::KPreSpace KPreSpace;
  typedef typename KPreSpace::Surfel PreSurfel;

 public:
  SurfelFollowers() requires concepts::CPreCellularGridSpaceND<KSpace> : myKSpace(&detail::preSpaces<KSpace>) {}
  SurfelFollowers(DGtal::ConstAlias<KSpace> ks) : myKSpace(&ks) {}

 
 public:
  

  void get(std::vector<Surfel>& result, const Surfel& aSurfel, const bool int2ext) {
    typename PSF::ResultArray preSurfels;
    PSF::get(preSurfels, aSurfel, int2ext);

    for (auto i = 0; i < 2; i++) {
      result[i] = std::nullopt;
      if (!preSurfels[i].has_value()) continue;

      Surfel surfel = myKSpace->sCell(result[i].value());
      if (!pred(surfel)) continue;
      
      result[i] = pred(surfel) ? surfel;
    }

    DGtal::SurfelAdjacency<KSpace::dimension> sa(int2ext);
    DGtal::SurfelNeighborhood<KSpace> sn;
    sn.init(myKSpace, &sa, aSurfel);

    DGtal::Dimension track_dir = *(myKSpace->sDirs(aSurfel));
    bool pos = myKSpace->sDirect(aSurfel, track_dir);

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
  }

  template <concepts::CSurfelPredicate TPredicate>
  void get(ResultArray& result, const Surfel& aSurfel, const bool int2ext, const TPredicate& pred) {
    get(result, aSurfel, int2ext);
    filter(result, pred);
  }


private:
  const KSpace* myKSpace;

  static auto getIsInsideFunctor(const KSpace* myKPtr) {
    typedef std::optional<typename KSpace::PreCellularGridSpace::Surfel> OptPreSurfel;
    return DGtal::functors::holdFunctor( [myKPtr](KSpace::SPreCell const& pt) { return signed_dist_to_unit_circle(pt); } );
  }

  class IsInsidePredicate
  template <concepts::CSurfelPredicate TPredicate>
  static void filter(ResultArray& result, const TPredicate& pred) {
    for (auto i = 0; i < 2; i++) {
      if (!result[i].has_value()) continue;
      if (!pred(result[i].value())) result[i] = std::nullopt;
    }
  }
};

}


template <concepts::CPreCellularGridSpaceND TKSpace, concepts::CSurfelPredicate TPredicate = TrueSurfelPredicate<TKSpace>>
class SurfelFollowers {
public:
  typedef TKSpace KSpace;
  typedef typename KSpace::Surfel Surfel;

public:
  ~SurfelFollowers();
  SurfelFollowers();
  SurfelFollowers(const SurfelFollowers &other)
      : myKPtr(other.myKPtr), mySurfel(other.mySurfel),
        myOrthDir(other.myOrthDir), myOrthDirect(other.myOrthDirect) {}
  SurfelFollowers &operator=(const SurfelFollowers &other) {
    if (this != &other) {
      myKPtr = other.myKPtr;
      mySurfel = other.mySurfel;
      myOrthDir = other.myOrthDir;
      myOrthDirect = other.myOrthDirect;
    }
    return *this;
  }
*/

