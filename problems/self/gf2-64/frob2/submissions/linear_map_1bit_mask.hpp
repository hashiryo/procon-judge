#pragma once
// 1 bit 刻みだが表を持たず、基底 64 個をマスクで AND する版。
#pragma GCC optimize("O3,unroll-loops")
#include "_shared/gf2-64/_common.hpp"
namespace gf2_64_frob2_1bit_mask {
// 基底の像 BASIS[i] = frob2(1 << i)。sq を呼ぶのはここだけで、表はこの XOR の
// 足し合わせ (倍々) で広げる。表の形が変わっても作り方は同じ。
constexpr auto BASIS= []() {
 auto spread= [](u32 a) constexpr -> u64 {
  u64 x= a;
  x= (x | (x << 16)) & 0x0000FFFF0000FFFFull;
  x= (x | (x << 8)) & 0x00FF00FF00FF00FFull;
  x= (x | (x << 4)) & 0x0F0F0F0F0F0F0F0Full;
  x= (x | (x << 2)) & 0x3333333333333333ull;
  x= (x | (x << 1)) & 0x5555555555555555ull;
  return x;
 };
 constexpr u8 RED[]= {0, 27, 45, 54, 90, 65, 119, 108};
 auto sq= [&](u64 a) constexpr -> u64 {
  u64 h= spread(u32(a >> 32));
  u64 d= h ^ (h << 1);
  return spread(u32(a)) ^ RED[h >> 60] ^ d ^ (d << 3);
 };
 array<u64, 64> b{};
 for(int i= 0; i < 64; ++i) b[i]= sq(sq(u64(1) << i));
 return b;
}();
// 表は基底 64 個 (512 B) だけ。ビットが立っているかを 0 か全 1 のマスクにして AND する
// ので、添字での表引きが無い。1 ビットあたり シフト + AND + 符号反転 + AND + XOR。
inline u64 frob2(u64 a) {
 u64 r= 0;
 for(int i= 0; i < 64; ++i) r^= BASIS[i] & -u64((a >> i) & 1);
 return r;
}
}  // namespace gf2_64_frob2_1bit_mask
inline vector<u64> run(const vector<u64>& as) {
 using gf2_64_frob2_1bit_mask::frob2;
 vector<u64> ans(as.size());
 for(size_t i= 0; i < as.size(); ++i) ans[i]= frob2(as[i]);
 return ans;
}
