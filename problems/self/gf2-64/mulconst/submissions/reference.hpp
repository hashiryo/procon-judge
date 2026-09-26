#pragma once
#include "common.hpp"
// 素朴 reference: bit ごとに shift して XOR し、x^64 以上を 1 bit ずつ落とす。
inline u64 mul_naive(u64 a, u64 b) {
 u64 lo= 0, hi= 0;
 for(int i= 0; i < 64; ++i)
  if((b >> i) & 1) {
   lo^= a << i;
   if(i) hi^= a >> (64 - i);
  }
 for(int i= 63; i >= 0; --i)
  if((hi >> i) & 1) {
   hi^= u64(1) << i;
   lo^= IRRED_LOW << i;
   if(i) hi^= IRRED_LOW >> (64 - i);
  }
 return lo;
}
inline vector<u64> run(const vector<u64>& as) {
 vector<u64> ans(as.size());
 for(size_t i= 0; i < as.size(); ++i) ans[i]= mul_naive(as[i], MUL_CONST);
 return ans;
}
