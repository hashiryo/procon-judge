#pragma once
// 1 bit 刻みの表 64 本 (1 KB) で引く版。T[i][0] は必ず 0 なので半分は無駄だが、
// 素直に 2 次元の表を引く形。引き 64 回、XOR 63 回。
#pragma GCC optimize("O3,unroll-loops")
#include "_shared/gf2-64/_common.hpp"
namespace gf2_64_frob2_1bit {
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
constexpr auto T= []() {
 array<array<u64, 2>, 64> t{};
 for(int p= 0; p < 64; ++p)
  for(int j= 0; j < 1; ++j) {
   const u64 v= BASIS[1 * p + j];
   for(int b= 0; b < (1 << j); ++b) t[p][(1 << j) | b]= t[p][b] ^ v;
  }
 return t;
}();
inline u64 frob2(u64 a) {
 return T[0][a & 1] ^ T[1][(a >> 1) & 1] ^ T[2][(a >> 2) & 1] ^ T[3][(a >> 3) & 1] ^
        T[4][(a >> 4) & 1] ^ T[5][(a >> 5) & 1] ^ T[6][(a >> 6) & 1] ^ T[7][(a >> 7) & 1] ^
        T[8][(a >> 8) & 1] ^ T[9][(a >> 9) & 1] ^ T[10][(a >> 10) & 1] ^ T[11][(a >> 11) & 1] ^
        T[12][(a >> 12) & 1] ^ T[13][(a >> 13) & 1] ^ T[14][(a >> 14) & 1] ^ T[15][(a >> 15) & 1] ^
        T[16][(a >> 16) & 1] ^ T[17][(a >> 17) & 1] ^ T[18][(a >> 18) & 1] ^ T[19][(a >> 19) & 1] ^
        T[20][(a >> 20) & 1] ^ T[21][(a >> 21) & 1] ^ T[22][(a >> 22) & 1] ^ T[23][(a >> 23) & 1] ^
        T[24][(a >> 24) & 1] ^ T[25][(a >> 25) & 1] ^ T[26][(a >> 26) & 1] ^ T[27][(a >> 27) & 1] ^
        T[28][(a >> 28) & 1] ^ T[29][(a >> 29) & 1] ^ T[30][(a >> 30) & 1] ^ T[31][(a >> 31) & 1] ^
        T[32][(a >> 32) & 1] ^ T[33][(a >> 33) & 1] ^ T[34][(a >> 34) & 1] ^ T[35][(a >> 35) & 1] ^
        T[36][(a >> 36) & 1] ^ T[37][(a >> 37) & 1] ^ T[38][(a >> 38) & 1] ^ T[39][(a >> 39) & 1] ^
        T[40][(a >> 40) & 1] ^ T[41][(a >> 41) & 1] ^ T[42][(a >> 42) & 1] ^ T[43][(a >> 43) & 1] ^
        T[44][(a >> 44) & 1] ^ T[45][(a >> 45) & 1] ^ T[46][(a >> 46) & 1] ^ T[47][(a >> 47) & 1] ^
        T[48][(a >> 48) & 1] ^ T[49][(a >> 49) & 1] ^ T[50][(a >> 50) & 1] ^ T[51][(a >> 51) & 1] ^
        T[52][(a >> 52) & 1] ^ T[53][(a >> 53) & 1] ^ T[54][(a >> 54) & 1] ^ T[55][(a >> 55) & 1] ^
        T[56][(a >> 56) & 1] ^ T[57][(a >> 57) & 1] ^ T[58][(a >> 58) & 1] ^ T[59][(a >> 59) & 1] ^
        T[60][(a >> 60) & 1] ^ T[61][(a >> 61) & 1] ^ T[62][(a >> 62) & 1] ^ T[63][(a >> 63) & 1];
}
}  // namespace gf2_64_frob2_1bit
struct GF2_64Op {
 static vector<u64> run(const vector<u64>& as) {
  using gf2_64_frob2_1bit::frob2;
  vector<u64> ans(as.size());
  for(size_t i= 0; i < as.size(); ++i) ans[i]= frob2(as[i]);
  return ans;
 }
};
