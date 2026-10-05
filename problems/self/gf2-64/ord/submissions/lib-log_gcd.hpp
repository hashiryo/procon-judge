// Library の GF2p64::log() から、位数 = (2^64-1) / gcd(log a, 2^64-1) とする。a = 1 なら log は 0 で、gcd が 2^64-1 になり
// 位数は 1。
#include "mylib/algebra/GF2p64.hpp"
#include <numeric>
#include <vector>
using namespace std;
using u64= unsigned long long;
inline vector<u64> run(const vector<u64>& as) {
 constexpr u64 M= ~0ull;
 vector<u64> ans(as.size());
 for(size_t i= 0; i < as.size(); ++i) ans[i]= M / gcd(GF2p64(as[i]).log(), M);
 return ans;
}
