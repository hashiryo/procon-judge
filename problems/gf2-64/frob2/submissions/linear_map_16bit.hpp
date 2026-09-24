#pragma once
// a^(2^2) を F_2-線型写像と見て、16 bit 刻みの表 4 本 (2 MB) で引く版。引きは 4 回、XOR は
// 3 回で済むが、表が L2 に収まらないのでランダムな入力では毎回遠くを叩くことになる。
#pragma GCC optimize("O3,unroll-loops")
#include "_shared/gf2-64/_common.hpp"
namespace gf2_64_frob2_16bit {
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
// 表は生配列で持つ (4 × 65536 要素あるので、std::array の operator[] だと
// constexpr の step 上限に当たる)。
struct Tab {
 u64 t[4][65536];
};
constexpr Tab T= []() {
 Tab r{};
 for(int p= 0; p < 4; ++p)
  for(int j= 0; j < 16; ++j) {
   const u64 v= BASIS[16 * p + j];
   for(int b= 0; b < (1 << j); ++b) r.t[p][(1 << j) | b]= r.t[p][b] ^ v;
  }
 return r;
}();
inline u64 frob2(u64 a) {
 return T.t[0][u16(a)] ^ T.t[1][u16(a >> 16)] ^ T.t[2][u16(a >> 32)] ^ T.t[3][u16(a >> 48)];
}
}  // namespace gf2_64_frob2_16bit
struct GF2_64Op {
 static vector<u64> run(const vector<u64>& as) {
  using gf2_64_frob2_16bit::frob2;
  vector<u64> ans(as.size());
  for(size_t i= 0; i < as.size(); ++i) ans[i]= frob2(as[i]);
  return ans;
 }
};
