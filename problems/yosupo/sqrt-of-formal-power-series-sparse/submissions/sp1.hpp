#pragma once
// 疎な冪級数の平方根 (sp1)。
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
// u = 1 + Σ_{k} c_k x^{d_k} の u^m を [lo, hi) で求める。v' u = m u' v から
// v_i = (Σ_k α_k v_{i-d_k}) / i - Σ_k c_k v_{i-d_k} (α_k = c_k (m+1) d_k)。v は 2M 未満で持つ。
template <int T> void seg(u32* v, const u32* iv, int lo, int hi, const int* d, const u32* c, const u32* al) {
 if(lo >= hi) return;
 if(d[0] == 1) {
  // v_i = (α_0 / i - c_0) v_{i-1} + (Σ_{k≥1} α_k v_{i-d_k}) / i - Σ_{k≥1} c_k v_{i-d_k}
  const u32 a0= al[0], q0= u32((u64(a0) << 32) / M), c0= 2 * M - c[0];
  u32 prev= v[lo - 1];
  for(int i= lo; i < hi; ++i) {
   u64 A= 0, B= 0;
   for(int k= 1; k < T; ++k) A+= u64(al[k]) * v[i - d[k]], B+= u64(c[k]) * v[i - d[k]];
   const u32 ivi= iv[i];
   u32 gm= shoup(ivi, a0, q0) + c0;  // 4M 未満
   gm= gm >= 2 * M ? gm - 2 * M : gm;
   u32 dl= red2(u64(red2(A)) * ivi) + M - u32(B % M);  // 3M 未満
   dl= dl >= 2 * M ? dl - 2 * M : dl;
   u32 r= red2(u64(prev) * gm) + dl;
   r= r >= 2 * M ? r - 2 * M : r;
   v[i]= prev= r;
  }
 } else {
  for(int i= lo; i < hi; ++i) {
   u64 A= 0, B= 0;
   for(int k= 0; k < T; ++k) A+= u64(al[k]) * v[i - d[k]], B+= u64(c[k]) * v[i - d[k]];
   u32 r= red2(u64(red2(A)) * iv[i]) + M - u32(B % M);
   v[i]= r >= 2 * M ? r - 2 * M : r;
  }
 }
}
// u^m の先頭 L 項 (m1 = m + 1 mod M)。f の項 (e_k, a_k) のうち k ≥ 1 を a_0 x^{e_0} で割って u を作る。
inline std::vector<u32> upow(const std::vector<std::pair<int, u32>>& f, int L, u32 m1) {
 std::vector<u32> v(L);
 v[0]= 1;
 const int e0= f[0].first;
 const u32 ia= powmod(f[0].second, M - 2);
 int d[10];
 u32 c[10], al[10];
 int t= 0;
 for(size_t k= 1; k < f.size(); ++k) {
  const int dk= f[k].first - e0;
  if(dk >= L) break;
  d[t]= dk, c[t]= mulmod(f[k].second, ia), al[t]= mulmod(mulmod(c[t], m1), u32(dk)), ++t;
 }
 if(t == 0) return v;
 const auto iv= inverses(L);
 for(int j= 1; j <= t; ++j) {
  const int lo= d[j - 1], hi= j == t ? L : d[j];
  dispatch(j, std::make_integer_sequence<int, 10>{}, [&](auto T) { seg<T>(v.data(), iv.data(), lo, hi, d, c, al); });
 }
 return v;
}
}  // namespace sp1
namespace sp1 {
// a の平方根 (無ければ -1)。M - 1 = 2^23 119、3 は原始根。
inline long long sqrt_mod(u32 a) {
 if(a == 0) return 0;
 if(powmod(a, (M - 1) / 2) != 1) return -1;
 int s= 23;
 u32 c= powmod(3, 119), r= powmod(a, 60), t= powmod(a, 119);
 while(t != 1) {
  int i= 0;
  for(u32 tt= t; tt != 1; tt= mulmod(tt, tt)) ++i;
  u32 b= c;
  for(int j= 0; j < s - i - 1; ++j) b= mulmod(b, b);
  s= i, c= mulmod(b, b), t= mulmod(t, c), r= mulmod(r, b);
 }
 return r;
}
}  // namespace sp1
// 疎な f の平方根。f = a_0 x^{e_0} u として、e_0 が偶数で a_0 が平方剰余なら sqrt(a_0) x^{e_0/2} u^{1/2}。
// u^{1/2} は u^m の漸化式で m = 1/2 (m + 1 = 3/2) としたもの。
inline vector<u32> run(int n, const vector<pair<int, u32>>& f) {
 using namespace sp1;
 vector<u32> g(n);
 if(f.empty()) return g;
 const int e0= f[0].first;
 if(e0 & 1) return {};
 const long long s= sqrt_mod(f[0].second);
 if(s < 0) return {};
 const int o= e0 / 2, L= n - o;
 const auto v= upow(f, L, mulmod(3, (M + 1) / 2));
 const u32 am= u32(s), q= u32((u64(am) << 32) / M);
 for(int i= 0; i < L; ++i) {
  u32 r= shoup(v[i], am, q);
  g[o + i]= r >= M ? r - M : r;
 }
 return g;
}
