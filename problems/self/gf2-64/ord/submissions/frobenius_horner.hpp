#pragma once
// frobenius.hpp の a^6700417 を、6700417 = 2^7·(3 + 2^3·(15 + 2^7·51)) + 1 と見て、Horner の形で a·F7(a^3·F3(a^15·F7(a^51)))
// と組んだ版。掛け算 (6 回) と Frobenius (5 回) の回数は同じで、sq が 1 回減り、frob16 と frob10 の代わりに frob7 と frob3 を引く。
// そのかわり掛け算が 1 本の鎖に並ぶので、1 つの元の中の待ちは長くなる。ほかは frobenius と同じ。
#include "_shared/gf2-64/_common.hpp"
#include "_shared/gf2-64/mul.hpp"
#include "_shared/gf2-64/sq.hpp"
#include "_shared/gf2-64/frob.hpp"
namespace gf2_64_ord_frob_horner {
using namespace gf2_64_pclmul;
inline u64 ord(u64 a) {
 const u64 n= mul(a, frob32(a)), fn= frob16(n), m= mul(n, fn);
 const u64 f8= frob8(m), t= mul(m, f8), f4= frob4(t), u= mul(t, f4), f2= frob2(u), v= mul(u, f2);
 const u64 c= mul(mul(a, frob7(a)), frob9(a));
 const u64 x3= mul(a, sq(a)), x15= mul(x3, frob2(x3)), x51= mul(x3, frob4(x3));
 const u64 d= mul(a, frob7(mul(x3, frob3(mul(x15, frob7(x51))))));
 return u64(v != 1 ? 3 : 1) * (f2 != u ? 5 : 1) * (f4 != t ? 17 : 1) * (f8 != m ? 257 : 1) * (fn != n ? 65537 : 1) * (frob32(d) != d ? 641 : 1) * (frob32(c) != c ? 6700417 : 1);
}
}  // namespace gf2_64_ord_frob_horner
inline vector<u64> run(const vector<u64>& as) {
 vector<u64> ans(as.size());
 for(size_t i= 0; i < as.size(); ++i) ans[i]= gf2_64_ord_frob_horner::ord(as[i]);
 return ans;
}
