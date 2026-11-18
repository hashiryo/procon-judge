#pragma once
// 素朴な実装 (intrinsics を使わない)。Horner 法の 1 歩は前の値を待つので、8 点を並べて回して待ちを隠す。点 x を掛けるのは
// x 2^32 / M の商を先に作る Shoup の方法 (掛け算 3 回、値は 2M 未満のまま回す)。
#include <vector>
namespace mpe_naive {
constexpr u32 M= 998244353;
}
inline vector<u32> run(const vector<u32>& f, const vector<u32>& x) {
 using namespace mpe_naive;
 const int n= int(f.size()), m= int(x.size());
 vector<u32> r(m);
 int j= 0;
 for(; j + 8 <= m; j+= 8) {
  u32 a[8]= {}, p[8], q[8];
  for(int k= 0; k < 8; ++k) p[k]= x[j + k], q[k]= u32((u64(p[k]) << 32) / M);
  for(int i= n - 1; i >= 0; --i) {
   const u32 c= f[i];
   for(int k= 0; k < 8; ++k) {
    u32 v= a[k] * p[k] - u32((u64(a[k]) * q[k]) >> 32) * M + c;  // 3M 未満
    a[k]= v >= 2 * M ? v - 2 * M : v;
   }
  }
  for(int k= 0; k < 8; ++k) r[j + k]= a[k] >= M ? a[k] - M : a[k];
 }
 for(; j < m; ++j) {
  const u32 p= x[j], q= u32((u64(p) << 32) / M);
  u32 a= 0;
  for(int i= n - 1; i >= 0; --i) {
   u32 v= a * p - u32((u64(a) * q) >> 32) * M + f[i];
   a= v >= 2 * M ? v - 2 * M : v;
  }
  r[j]= a >= M ? a - M : a;
 }
 return r;
}
