#include "neo/base/core/nt/factors.hpp"

constexpr auto f= Factors(2 * 2 * 3 * 5);
static_assert(f.size() == 3);
static_assert(f[0].first == 2);
static_assert(f[0].second == 2);
static_assert(f[1].first == 3);
static_assert(f[1].second == 1);
static_assert(f[2].first == 5);
static_assert(f[2].second == 1);
// 昇順に並ぶ (rho が見つける順ではない)
constexpr auto g= Factors(7 * 2 * 2 * 3);
static_assert(g[0].first == 2 && g[0].second == 2);
static_assert(g[1].first == 3 && g[1].second == 1);
static_assert(g[2].first == 7 && g[2].second == 1);
// 2 つの素数の積 (rho の道)
constexpr auto h= Factors(1000000007ull * 1000000009ull);
static_assert(h.size() == 2);
static_assert(h[0].first == 1000000007ull && h[0].second == 1);
static_assert(h[1].first == 1000000009ull && h[1].second == 1);
constexpr int n= totient(100);
static_assert(n == 40);
constexpr auto m= lambda_like(998244353ull);  // p - 1 = 2^23 * 7 * 17
static_assert(m == 998244352ull);
signed main() { return 0; }
