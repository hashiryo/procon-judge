#pragma once
// 疎な冪級数の log (sp1)。
#ifdef __x86_64__
#include <immintrin.h>
#else
#include <simde/x86/avx2.h>
#endif
#include <algorithm>
#include <utility>
#include <vector>
namespace sp1 {
constexpr u32 M= 998244353;
constexpr u32 MI= [] {  // -M^{-1} mod 2^32
 u32 x= M;
 for(int i= 0; i < 5; ++i) x*= 2 - M * x;
 return -x;
}();
constexpr u64 BX= ~u64(0) / M;
inline u32 mulmod(u32 a, u32 b) { return u32(u64(a) * b % M); }
// 2^62 未満の t を 2M 未満に (Barrett)。
inline u32 red2(u64 t) { return u32(t - u64((__uint128_t(t) * BX) >> 64) * M); }
inline u32 powmod(u32 a, u64 e) {
 u32 r= 1;
 for(; e; e>>= 1, a= mulmod(a, a))
  if(e & 1) r= mulmod(r, a);
 return r;
}
// a < 2^32 に x を掛けて 2M 未満で返す (q = floor(x 2^32 / M))。
inline u32 shoup(u32 a, u32 x, u32 q) { return a * x - u32((u64(a) * q) >> 32) * M; }
// x y 2^{-32} mod M を 8 個ずつ。x、y が 2M 未満なら結果も 2M 未満。
inline __m256i mont8(__m256i x, __m256i y) {
 const __m256i mv= _mm256_set1_epi32(M), mi= _mm256_set1_epi32(MI);
 __m256i te= _mm256_mul_epu32(x, y), to= _mm256_mul_epu32(_mm256_srli_epi64(x, 32), _mm256_srli_epi64(y, 32));
 __m256i ue= _mm256_add_epi64(te, _mm256_mul_epu32(_mm256_mul_epu32(te, mi), mv));
 __m256i uo= _mm256_add_epi64(to, _mm256_mul_epu32(_mm256_mul_epu32(to, mi), mv));
 return _mm256_blend_epi32(_mm256_srli_epi64(ue, 32), uo, 0b10101010);
}
// iv[i] = 1/i mod M (1 ≤ i < n、iv[0] = 0)。レーン b が i ≡ b + 1 (mod 8) の数の累積積を受け持つ 8 本の鎖で、
// 前向きに累積積 (Montgomery の積なので 2^{-32} が 1 回ずつ掛かる) を置き、レーンごとの総積の逆元から後ろ向きに戻す。
// 2^{-32} の数は前向きと後ろ向きで打ち消し合うので、変換なしで 1/i がそのまま出る。
inline std::vector<u32> inverses(int n) {
 if(n <= 1) return std::vector<u32>(std::max(n, 0));
 const int steps= (n - 1 + 7) / 8;
 std::vector<u32> iv(8 * steps + 8);
 __m256i acc= _mm256_set1_epi32(1), idx= _mm256_setr_epi32(1, 2, 3, 4, 5, 6, 7, 8);
 const __m256i eight= _mm256_set1_epi32(8), mv= _mm256_set1_epi32(M);
 for(int t= 0; t < steps; ++t) {
  acc= mont8(acc, idx);
  _mm256_storeu_si256((__m256i*)(iv.data() + 1 + 8 * t), acc);
  idx= _mm256_add_epi32(idx, eight);
 }
 alignas(32) u32 a[8];
 _mm256_store_si256((__m256i*)a, acc);
 for(int b= 0; b < 8; ++b) a[b]= powmod(a[b] % M, M - 2);
 __m256i cur= _mm256_load_si256((const __m256i*)a);
 idx= _mm256_sub_epi32(idx, eight);
 for(int t= steps - 1; t >= 0; --t) {
  __m256i pre= t ? _mm256_loadu_si256((const __m256i*)(iv.data() + 1 + 8 * (t - 1))) : _mm256_set1_epi32(1);
  __m256i r= mont8(cur, pre);
  _mm256_storeu_si256((__m256i*)(iv.data() + 1 + 8 * t), _mm256_min_epu32(r, _mm256_sub_epi32(r, mv)));
  cur= mont8(cur, idx);
  idx= _mm256_sub_epi32(idx, eight);
 }
 iv.resize(n), iv[0]= 0;
 return iv;
}
template <int... Ts, class F> void dispatch(int t, std::integer_sequence<int, Ts...>, F&& f) { ((t == Ts ? f(std::integral_constant<int, Ts>{}) : void()), ...); }
}  // namespace sp1
namespace sp1 {
// [lo, hi) の h を h_i = (h_i の初期値) + Σ_k w_k h_{i - e_k} で求める (h は M 未満)。
template <int T> void seg(u32* h, int lo, int hi, const int* e, const u32* w) {
 if(lo >= hi) return;
 if(e[0] == 1) {
  const u32 w0= w[0], q0= u32((u64(w0) << 32) / M);
  u32 prev= h[lo - 1];
  for(int i= lo; i < hi; ++i) {
   u64 rest= h[i];
   for(int k= 1; k < T; ++k) rest+= u64(w[k]) * h[i - e[k]];
   u32 r= u32(rest % M) + shoup(prev, w0, q0);  // 3M 未満
   r= r >= 2 * M ? r - 2 * M : r;
   r= r >= M ? r - M : r;
   h[i]= prev= r;
  }
 } else {
  for(int i= lo; i < hi; ++i) {
   u64 acc= h[i];
   for(int k= 0; k < T; ++k) acc+= u64(w[k]) * h[i - e[k]];
   h[i]= u32(acc % M);
  }
 }
}
}  // namespace sp1
// 疎な f (定数項 1) の log。f g' = f' から h_i = i g_i が h_i = i f_i - Σ_{k≥1} a_k h_{i - e_k} を満たすので、
// h を前から求めてから 1/i を掛ける。1/i の表は 8 本の鎖でまとめて求め、掛けるのも 8 個ずつ。
inline vector<u32> run(int n, const vector<pair<int, u32>>& f) {
 using namespace sp1;
 vector<u32> h(n);
 int e[10];
 u32 w[10];
 int t= 0;
 for(size_t k= 1; k < f.size(); ++k) {
  if(f[k].first >= n) break;
  e[t]= f[k].first, w[t]= M - f[k].second, h[e[t]]= mulmod(u32(e[t]), f[k].second), ++t;
 }
 if(t == 0) return h;
 for(int j= 1; j <= t; ++j) {
  const int lo= e[j - 1], hi= j == t ? n : e[j];
  dispatch(j, std::make_integer_sequence<int, 10>{}, [&](auto T) { seg<T>(h.data(), lo, hi, e, w); });
 }
 const auto iv= inverses(n);
 const __m256i r2= _mm256_set1_epi32(u32((u64(1) << 63) % M * 2 % M)), mv= _mm256_set1_epi32(M);
 int i= 0;
 for(; i + 8 <= n; i+= 8) {
  __m256i x= mont8(mont8(_mm256_loadu_si256((const __m256i*)(h.data() + i)), _mm256_loadu_si256((const __m256i*)(iv.data() + i))), r2);
  _mm256_storeu_si256((__m256i*)(h.data() + i), _mm256_min_epu32(x, _mm256_sub_epi32(x, mv)));
 }
 for(; i < n; ++i) h[i]= mulmod(h[i], iv[i]);
 return h;
}
