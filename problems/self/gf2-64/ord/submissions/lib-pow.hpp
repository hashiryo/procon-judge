// Library の GF2p64::pow で、素数 p ごとに a^((2^64-1)/p) を求め、1 でない p を掛ける (冪 7 回)。
#include "mylib/algebra/GF2p64.hpp"
#include <vector>
using namespace std;
using u64= unsigned long long;
inline vector<u64> run(const vector<u64>& as) {
 constexpr u64 M= ~0ull, PR[7]= {3, 5, 17, 257, 641, 65537, 6700417};
 vector<u64> ans(as.size());
 for(size_t i= 0; i < as.size(); ++i) {
  u64 o= 1;
  for(u64 p: PR)
   if(GF2p64(as[i]).pow(M / p) != GF2p64(1)) o*= p;
  ans[i]= o;
 }
 return ans;
}
