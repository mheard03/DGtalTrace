#include <cstddef>
namespace DGtal {
#ifdef WITH_BIGINTEGER
  typedef BigInteger SafePtrDiff;
#else
  typedef std::ptrdiff_t SafePtrDiff;
#endif
}