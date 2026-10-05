#pragma once
// 素数 p ごとに a^((2^64-1)/p) == 1 かを、離散対数も表も使わずに、Frobenius (byte の表を引く線型写像) と掛け算だけで判定する。
// どの判定も「ある元が部分体に入っているか (Frobenius で動かないか)」に言い換えられる。
//   65537: n = a^(1+2^32) (GF(2^32) へのノルム) が GF(2^16) に入るか (frob16(n) == n)
//   257、17、5、3: m = n^(1+2^16) (GF(2^16) へのノルム) を GF(2^8)、GF(2^4)、GF(2^2) へのノルムに順に落とし、m が GF(2^8) に、
//                  t = m^257 が GF(2^4) に、u = t^17 が GF(2^2) に入るか、v = u^5 が 1 か
//   6700417: a^641 (641 = 2^9 + 2^7 + 1) が GF(2^32) に入るか
//   641: a^6700417 (6700417 = 51·2^17 + 15·2^10 + 3·2^7 + 1、a^3、a^15、a^51 から組む) が GF(2^32) に入るか
#include "_shared/gf2-64/_common.hpp"
#include "_shared/gf2-64/mul.hpp"
#include "_shared/gf2-64/sq.hpp"
#include "_shared/gf2-64/frob.hpp"
namespace gf2_64_ord_frob {
using namespace gf2_64_pclmul;
inline u64 ord(u64 a) {
 const u64 n= mul(a, frob32(a)), fn= frob16(n), m= mul(n, fn);
 const u64 f8= frob8(m), t= mul(m, f8), f4= frob4(t), u= mul(t, f4), f2= frob2(u), v= mul(u, f2);
 const u64 c= mul(mul(a, frob7(a)), frob9(a));
 const u64 x3= mul(a, sq(a)), x15= mul(x3, frob2(x3)), x51= mul(x3, frob4(x3));
 const u64 d= mul(mul(sq(frob16(x51)), frob10(x15)), mul(frob7(x3), a));
 return u64(v != 1 ? 3 : 1) * (f2 != u ? 5 : 1) * (f4 != t ? 17 : 1) * (f8 != m ? 257 : 1) * (fn != n ? 65537 : 1) * (frob32(d) != d ? 641 : 1) * (frob32(c) != c ? 6700417 : 1);
}
}  // namespace gf2_64_ord_frob
inline vector<u64> run(const vector<u64>& as) {
 vector<u64> ans(as.size());
 for(size_t i= 0; i < as.size(); ++i) ans[i]= gf2_64_ord_frob::ord(as[i]);
 return ans;
}
