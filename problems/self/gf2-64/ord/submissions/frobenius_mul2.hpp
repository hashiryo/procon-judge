#pragma once
// frobenius.hpp の 13 回の掛け算のうち、同じ段で互いを待たないものを 2 つずつ mul2 (__m256i の 2 lane の掛け算) にまとめた版。
// 1 つの元の中だけでまとめ、元をまたいではまとめない。(n, a^3)、(m, a^15)、(a^51, a·F7(a^3))、(t, F17(a^51)·F10(a^15))、
// (u, a^6700417) の 5 組を mul2 で、残りの 3 回 (a·F7(a)、a^641、v) をスカラで掛ける。mul2 は Library の GF2p64 と同じく、
// x86 では VPCLMULQDQ があれば 1 命令で、無ければ (arm も) 64 bit の clmul を 2 回使う。ほかは frobenius と同じ。
#include "_shared/gf2-64/_common.hpp"
#include "_shared/gf2-64/mul.hpp"
#include "_shared/gf2-64/sq.hpp"
#include "_shared/gf2-64/frob.hpp"
namespace gf2_64_ord_frob_mul2 {
using namespace gf2_64_pclmul;
template <bool V> inline __m256i mul2(const __m256i& a_vec, const __m256i& b_vec) {
 const __m256i RED256= _mm256_setr_epi8(0, 27, 45, 54, 90, 65, 119, 108, 0, 0, 0, 0, 0, 0, 0, 0, 0, 27, 45, 54, 90, 65, 119, 108, 0, 0, 0, 0, 0, 0, 0, 0);
 __m256i prod;
 if constexpr(V) prod= _mm256_clmulepi64_epi128(a_vec, b_vec, 0);
 else prod= _mm256_setr_m128i(_mm_clmulepi64_si128(_mm256_castsi256_si128(a_vec), _mm256_castsi256_si128(b_vec), 0), _mm_clmulepi64_si128(_mm256_extracti128_si256(a_vec, 1), _mm256_extracti128_si256(b_vec, 1), 0));
 __m256i h= _mm256_srli_si256(prod, 8);
 __m256i d= _mm256_xor_si256(h, _mm256_slli_epi64(h, 1));
 return _mm256_xor_si256(_mm256_xor_si256(prod, _mm256_shuffle_epi8(RED256, _mm256_srli_epi64(h, 60))), _mm256_xor_si256(d, _mm256_slli_epi64(d, 3)));
}
inline std::pair<u64, u64> unpack(const __m256i& v) { return {u64(_mm256_extract_epi64(v, 0)), u64(_mm256_extract_epi64(v, 2))}; }
inline __m256i pack(u64 lo, u64 hi) { return _mm256_set_epi64x(0, (long long)hi, 0, (long long)lo); }
template <bool V> inline u64 ord(u64 a) {
 const u64 f7a= frob7(a), f9a= frob9(a);
 const auto [n, x3]= unpack(mul2<V>(pack(a, a), pack(frob32(a), sq(a))));
 const u64 c1= mul(a, f7a), fn= frob16(n);
 const auto [m, x15]= unpack(mul2<V>(pack(n, x3), pack(fn, frob2(x3))));
 const auto [x51, e]= unpack(mul2<V>(pack(x3, a), pack(frob4(x3), frob7(x3))));
 const u64 c= mul(c1, f9a), f8= frob8(m);
 const auto [t, g]= unpack(mul2<V>(pack(m, sq(frob16(x51))), pack(f8, frob10(x15))));
 const u64 f4= frob4(t);
 const auto [u, d]= unpack(mul2<V>(pack(t, g), pack(f4, e)));
 const u64 f2= frob2(u), v= mul(u, f2);
 return u64(v != 1 ? 3 : 1) * (f2 != u ? 5 : 1) * (f4 != t ? 17 : 1) * (f8 != m ? 257 : 1) * (fn != n ? 65537 : 1) * (frob32(d) != d ? 641 : 1) * (frob32(c) != c ? 6700417 : 1);
}
template <bool V> inline void solve_all(const vector<u64>& as, vector<u64>& ans) {
 for(size_t i= 0; i < as.size(); ++i) ans[i]= ord<V>(as[i]);
}
}  // namespace gf2_64_ord_frob_mul2
inline vector<u64> run(const vector<u64>& as) {
 vector<u64> ans(as.size());
#ifdef __x86_64__
 if(__builtin_cpu_supports("vpclmulqdq")) return gf2_64_ord_frob_mul2::solve_all<1>(as, ans), ans;
#endif
 return gf2_64_ord_frob_mul2::solve_all<0>(as, ans), ans;
}
