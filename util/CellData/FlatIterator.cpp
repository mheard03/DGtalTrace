#include <boost/container/static_vector.hpp>
#include <iterator>
#include <algorithm>   // std::upper_bound, std::distance

template <typename T, std::size_t InnerCap, std::size_t OuterCap>
class FlatStaticIterator {
public:
    using iterator_category = std::random_access_iterator_tag;
    using value_type        = T;
    using difference_type   = std::ptrdiff_t;
    using pointer           = const T*;
    using reference         = const T&;

private:
    using Inner = boost::container::static_vector<T, InnerCap>;
    using Outer = boost::container::static_vector<Inner, OuterCap>;

    // Prefix-sum table — fits entirely on the stack
    // Entry k = sum of sizes of inner vectors [0, k)
    // Size is OuterCap+1 worst-case; we store only outer.size()+1 live entries
    using PrefixArray = std::array<std::ptrdiff_t, OuterCap + 1>;

    const Outer*       myOuter;
    const PrefixArray* myPrefixSums;
    std::size_t        myLiveCount;   // number of live prefix-sum entries
    std::ptrdiff_t     myIndex;       // flat index

    void locate(std::ptrdiff_t flat,
                std::size_t&  outerIdx,
                std::size_t&  innerIdx) const
    {
        auto first = myPrefixSums->begin();
        auto last  = first + myLiveCount;
        auto it    = std::upper_bound(first, last, flat);
        --it;
        outerIdx = static_cast<std::size_t>(std::distance(first, it));
        innerIdx = static_cast<std::size_t>(flat - *it);
    }

public:
    FlatStaticIterator() = default;

    FlatStaticIterator(const Outer*       outer,
                       const PrefixArray* prefixSums,
                       std::size_t        liveCount,
                       std::ptrdiff_t     index)
        : myOuter(outer)
        , myPrefixSums(prefixSums)
        , myLiveCount(liveCount)
        , myIndex(index)
    {}

    // --- Element access ---

    reference operator*() const {
        std::size_t o, i;
        locate(myIndex, o, i);
        return (*myOuter)[o][i];
    }
    pointer   operator->() const { return &**this; }
    reference operator[](difference_type n) const {
        std::size_t o, i;
        locate(myIndex + n, o, i);
        return (*myOuter)[o][i];
    }

    // --- Increment / decrement ---

    FlatStaticIterator& operator++()    { ++myIndex; return *this; }
    FlatStaticIterator& operator--()    { --myIndex; return *this; }
    FlatStaticIterator  operator++(int) { auto t = *this; ++*this; return t; }
    FlatStaticIterator  operator--(int) { auto t = *this; --*this; return t; }

    // --- Arithmetic ---

    FlatStaticIterator& operator+=(difference_type n) { myIndex += n; return *this; }
    FlatStaticIterator& operator-=(difference_type n) { myIndex -= n; return *this; }

    friend FlatStaticIterator operator+(FlatStaticIterator it, difference_type n) { it += n; return it; }
    friend FlatStaticIterator operator+(difference_type n, FlatStaticIterator it) { it += n; return it; }
    friend FlatStaticIterator operator-(FlatStaticIterator it, difference_type n) { it -= n; return it; }

    friend difference_type operator-(const FlatStaticIterator& a,
                                     const FlatStaticIterator& b)
    { return a.myIndex - b.myIndex; }

    // --- Comparisons ---

    friend auto operator<=>(const FlatStaticIterator&,
                             const FlatStaticIterator&) = default;
    friend bool operator==(const FlatStaticIterator&,
                            const FlatStaticIterator&)  = default;
};