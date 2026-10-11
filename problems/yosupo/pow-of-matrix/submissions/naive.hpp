#pragma once
#include "../common.hpp"
// 二分累乗。積は i-k-j の順で u64 にためて、8 項ごとに M で割った余りに直す。
namespace naive {
inline vector<u32> mul(int n, const vector<u32>& A, const vector<u32>& B) {
 vector<u32> C((size_t)n * n);
 vector<u64> acc(n);
 for(int i= 0; i < n; ++i) {
  fill(acc.begin(), acc.end(), 0);
  for(int k= 0; k < n; ++k) {
   const u64 x= A[(size_t)i * n + k];
   const u32* b= &B[(size_t)k * n];
   for(int j= 0; j < n; ++j) acc[j]+= x * b[j];
   if((k & 7) == 7)
    for(int j= 0; j < n; ++j) acc[j]%= MOD;
  }
  for(int j= 0; j < n; ++j) C[(size_t)i * n + j]= u32(acc[j] % MOD);
 }
 return C;
}
}  // namespace naive
inline vector<u32> run(int N, u64 K, const vector<u32>& a) {
 vector<u32> res((size_t)N * N), base= a;
 for(int i= 0; i < N; ++i) res[(size_t)i * N + i]= 1;
 for(; K; K>>= 1) {
  if(K & 1) res= naive::mul(N, res, base);
  if(K > 1) base= naive::mul(N, base, base);
 }
 return res;
}
