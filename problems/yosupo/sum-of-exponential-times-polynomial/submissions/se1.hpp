#pragma once
// Σ r^i i^d (se1)。
#ifdef __x86_64__
#include <immintrin.h>
#else
#include <simde/x86/avx2.h>
#endif
#include <algorithm>
#include <cmath>
#include <vector>
namespace se1 {
constexpr u32 M= 998244353;
constexpr u32 MI= [] {  // -M^{-1} mod 2^32
 u32 x= M;
 for(int i= 0; i < 5; ++i) x*= 2 - M * x;
 return -x;
}();
inline u32 mul(u32 a, u32 b) { return u32(u64(a) * b % M); }
inline u32 add(u32 a, u32 b) { return a + b >= M ? a + b - M : a + b; }
inline u32 sub(u32 a, u32 b) { return a >= b ? a - b : a + M - b; }
inline u32 pw(u32 a, u64 e) {
 u32 r= 1;
 for(; e; e>>= 1, a= mul(a, a))
  if(e & 1) r= mul(r, a);
 return r;
}
// x y 2^{-32} mod M を 8 個ずつ。x、y が 2M 未満なら結果も 2M 未満。
inline __m256i mont8(__m256i x, __m256i y) {
 const __m256i mv= _mm256_set1_epi32(M), mi= _mm256_set1_epi32(MI);
 __m256i te= _mm256_mul_epu32(x, y), to= _mm256_mul_epu32(_mm256_srli_epi64(x, 32), _mm256_srli_epi64(y, 32));
 __m256i ue= _mm256_add_epi64(te, _mm256_mul_epu32(_mm256_mul_epu32(te, mi), mv));
 __m256i uo= _mm256_add_epi64(to, _mm256_mul_epu32(_mm256_mul_epu32(to, mi), mv));
 return _mm256_blend_epi32(_mm256_srli_epi64(ue, 32), uo, 0b10101010);
}
// out[i] = 1/a[i] (a[i] は 1 以上 M 未満)。レーン b が添字 ≡ b (mod 8) の累積積を受け持つ 8 本の鎖で、前向きに累積積を
// 置き、レーンごとの総積の逆元から後ろ向きに戻す (Montgomery の 2^{-32} は前向きと後ろ向きで打ち消し合う)。
inline u32 inv_all(const u32* a, u32* out, int n) {
 const int steps= n / 8;
 const __m256i mv= _mm256_set1_epi32(M);
 u32 prod= 1;
 if(steps) {
  __m256i acc= _mm256_set1_epi32(1);
  for(int t= 0; t < steps; ++t) {
   acc= mont8(acc, _mm256_loadu_si256((const __m256i*)(a + 8 * t)));
   _mm256_storeu_si256((__m256i*)(out + 8 * t), acc);
  }
  alignas(32) u32 c[8];
  _mm256_store_si256((__m256i*)c, acc);
  // レーンの総積は (Π a) 2^{-32 steps} なので、全体の積は 8 本の積に 2^{256 steps} を掛けたもの。
  for(int b= 0; b < 8; ++b) c[b]%= M, prod= mul(prod, c[b]), c[b]= pw(c[b], M - 2);
  prod= mul(prod, pw(u32((u64(1) << 32) % M), u64(8) * steps));
  __m256i cur= _mm256_load_si256((const __m256i*)c);
  for(int t= steps - 1; t >= 0; --t) {
   __m256i pre= t ? _mm256_loadu_si256((const __m256i*)(out + 8 * (t - 1))) : _mm256_set1_epi32(1);
   __m256i r= mont8(cur, pre);
   cur= mont8(cur, _mm256_loadu_si256((const __m256i*)(a + 8 * t)));
   _mm256_storeu_si256((__m256i*)(out + 8 * t), _mm256_min_epu32(r, _mm256_sub_epi32(r, mv)));
  }
 }
 for(int i= 8 * steps; i < n; ++i) prod= mul(prod, a[i]), out[i]= pw(a[i], M - 2);
 return prod;
}
// out[i] = Π_{j≤i} a[j]。8 個の区間に分けて区間ごとの累積積を並べて求め (鎖が 8 本になる)、あとで前の区間の積を掛ける。
inline void prefix_prod(const u32* a, u32* out, int n) {
 if(n <= 0) return;
 constexpr int S= 8;
 const int len= (n + S - 1) / S;
 u32 acc[S];
 std::fill(acc, acc + S, 1u);
 const int full= n / len;  // 区間の長さが len のものの数
 for(int t= 0; t < len; ++t) {
  for(int s= 0; s < full; ++s) {
   const int i= s * len + t;
   out[i]= acc[s]= mul(acc[s], a[i]);
  }
  if(full < S) {
   const int i= full * len + t;
   if(i < n) out[i]= acc[full]= mul(acc[full], a[i]);
  }
 }
 u32 carry= 1;
 for(int s= 0; s * len < n; ++s) {
  const int lo= s * len, hi= std::min(n, lo + len);
  if(s)
   for(int i= lo; i < hi; ++i) out[i]= mul(out[i], carry);
  carry= mul(carry, acc[s]);
 }
}
// t[i] = i^d (0 ≤ i ≤ D、0^0 = 1)。√D 以下の素数で区画ごとに篩い、合成数 m には最小の素因数 p について m/p と p^d を
// 記録して t[m] = t[m/p] p^d とする (素数は大きい順に書いて最後に書いた最小の素数を残す)。素数は冪を直接求める。
inline std::vector<u32> pow_table(int D, u32 d) {
 std::vector<u32> t(D + 1);
 if(d == 0) return std::fill(t.begin(), t.end(), 1u), t;
 if(D >= 1) t[1]= 1;
 int sq= int(std::sqrt(double(D)));
 while(sq * sq > D) --sq;
 while((sq + 1) * (sq + 1) <= D) ++sq;
 std::vector<char> comp(sq + 1);
 std::vector<int> ps;
 std::vector<u32> pp;
 for(int i= 2; i <= sq; ++i)
  if(!comp[i]) {
   ps.push_back(i), pp.push_back(pw(i, d));
   for(int j= i * i; j <= sq; j+= i) comp[j]= 1;
  }
 constexpr int B= 1 << 15;
 std::vector<u32> cof(B), ppv(B);
 for(int L= 2; L <= D; L+= B) {
  const int R= std::min(D + 1, L + B);
  std::fill(cof.begin(), cof.begin() + (R - L), 0u);
  for(int j= int(ps.size()) - 1; j >= 0; --j) {
   const int p= ps[j];
   const long long st= std::max((long long)p * p, (long long)(L + p - 1) / p * p);
   if(st >= R) continue;
   u32 k= u32(st / p);
   const u32 v= pp[j];
   for(int m= int(st); m < R; m+= p, ++k) cof[m - L]= k, ppv[m - L]= v;
  }
  for(int m= L; m < R; ++m) {
   const u32 c= cof[m - L];
   t[m]= c ? mul(t[c], ppv[m - L]) : pw(u32(m), d);
  }
 }
 return t;
}
// C(N, k) (0 ≤ k ≤ N) と N!。比 (N - k + 1)/k の累積積で、1/k はまとめて求め、そのときの積が N!。
struct Binom {
 std::vector<u32> c;
 u32 fact;
};
inline Binom binom_row(int N) {
 Binom b;
 b.c.assign(N + 1, 0), b.c[0]= 1, b.fact= 1;
 if(N == 0) return b;
 std::vector<u32> ks(N), ik(N);
 for(int k= 1; k <= N; ++k) ks[k - 1]= u32(k);
 b.fact= inv_all(ks.data(), ik.data(), N);
 for(int k= 1; k <= N; ++k) ks[k - 1]= mul(u32(N - k + 1), ik[k - 1]);
 prefix_prod(ks.data(), b.c.data() + 1, N);
 return b;
}
// y[0..K) を 0..K-1 での値とする K-1 次以下の多項式の x での値 (bn は binom_row(K - 1))。
// f(x) = P/(K-1)! Σ_k y_k (-1)^{K-1-k} C(K-1, k) / (x - k)、P = Π_j (x - j)。x が 0..K-1 のどれかと合同なら y を返す。
inline u32 lagrange_at(const std::vector<u32>& y, u64 xx, const Binom& bn) {
 const int K= int(y.size());
 if(xx < u64(K)) return y[xx];
 const u32 x= u32(xx % M);
 if(x < u32(K)) return y[x];
 std::vector<u32> den(K), iden(K);
 for(int k= 0; k < K; ++k) den[k]= sub(x, u32(k));
 const u32 P= inv_all(den.data(), iden.data(), K);
 // 項 y_k C(K-1,k) / (x-k) を 8 個ずつ Montgomery の積 2 回で作り (2^{-64} が掛かる)、符号は k の偶奇で決まるので
 // レーンごとに固定の印で裏返し、u64 の 2 本にためる。
 const __m256i mv2= _mm256_set1_epi32(2 * M), lo32= _mm256_set1_epi64x(0xffffffffll);
 const int par= (K - 1) & 1;  // (K-1-k) が奇数のレーン
 const __m256i neg= par ? _mm256_setr_epi32(-1, 0, -1, 0, -1, 0, -1, 0) : _mm256_setr_epi32(0, -1, 0, -1, 0, -1, 0, -1);
 __m256i ae= _mm256_setzero_si256(), ao= _mm256_setzero_si256();
 int k= 0;
 for(; k + 8 <= K; k+= 8) {
  __m256i w= mont8(mont8(_mm256_loadu_si256((const __m256i*)(y.data() + k)), _mm256_loadu_si256((const __m256i*)(bn.c.data() + k))), _mm256_loadu_si256((const __m256i*)(iden.data() + k)));
  w= _mm256_blendv_epi8(w, _mm256_sub_epi32(mv2, w), neg);
  ae= _mm256_add_epi64(ae, _mm256_and_si256(w, lo32)), ao= _mm256_add_epi64(ao, _mm256_srli_epi64(w, 32));
 }
 alignas(32) u64 t[4];
 _mm256_store_si256((__m256i*)t, _mm256_add_epi64(ae, ao));
 const u32 r2= u32((u64(1) << 63) % M * 2 % M);  // 2^64 mod M。Montgomery の積 2 回の 2^{-64} を戻す
 u32 s= mul(u32((t[0] % M + t[1] % M + t[2] % M + t[3] % M) % M), r2);
 for(; k < K; ++k) {
  const u32 w= mul(mul(y[k], bn.c[k]), iden[k]);
  s= ((K - 1 - k) & 1) ? sub(s, w) : add(s, w);
 }
 return mul(mul(P, pw(bn.fact, M - 2)), s);
}
}  // namespace se1
namespace se1 {
// S(k) = Σ_{i<k} r^i i^d (0 ≤ k ≤ d + 1、r ≠ 0, 1) から、S(x) = c + r^x g(x) (g は d 次以下) の c と g(0..d) を求める。
// g の d + 1 階差分が 0 なので c = r^{d+1} Σ_k C(d+1, k) (-1)^{d+1-k} r^{-k} S(k) / (1 - r)^{d+1}。
// C(d+1, k) = C(d, k) + C(d, k-1) なので、補間で使う C(d, k) の行だけを作る。
inline u32 const_term(const std::vector<u32>& S, u32 r, const Binom& bn, std::vector<u32>* g) {
 const int D= int(S.size()) - 2;
 const u32 ir= pw(r, M - 2), ir4= pw(ir, 4);
 u32 q[4]= {1, ir, mul(ir, ir), mul(mul(ir, ir), ir)};  // r^{-k} を 4 本の鎖で
 u64 sp= 0, sn= 0;
 std::vector<u32> rk(D + 2);
 for(int k= 0; k <= D + 1; ++k) {
  rk[k]= q[k & 3], q[k & 3]= mul(q[k & 3], ir4);
  const u32 cb= add(k <= D ? bn.c[k] : 0, k ? bn.c[k - 1] : 0);
  const u32 w= mul(mul(cb, rk[k]), S[k]);
  if((D + 1 - k) & 1) sn+= w;
  else sp+= w;
 }
 const u32 num= sub(u32(sp % M), u32(sn % M));
 const u32 c= mul(mul(num, pw(r, D + 1)), pw(pw(sub(1, r), D + 1), M - 2));
 if(g) {
  g->resize(D + 1);
  for(int k= 0; k <= D; ++k) (*g)[k]= mul(sub(S[k], c), rk[k]);
 }
 return c;
}
// S(0..d+1)。r^i は 4 本の鎖で作る。
inline std::vector<u32> prefix_sums(const std::vector<u32>& t, u32 r) {
 const int D= int(t.size()) - 1;
 std::vector<u32> S(D + 2);
 const u32 r4= pw(r, 4);
 u32 q[4]= {1, r, mul(r, r), mul(mul(r, r), r)};
 for(int i= 0; i <= D; ++i) S[i + 1]= add(S[i], mul(q[i & 3], t[i])), q[i & 3]= mul(q[i & 3], r4);
 return S;
}
}  // namespace se1
// Σ_{i<n} r^i i^d。r = 1 なら d+1 次の多項式 F(x) = Σ_{i<x} i^d を 0..d+1 の値から補間し、r ≠ 0, 1 なら
// S(x) = c + r^x g(x) の c と g(0..d) を求めて g(n) を補間する (min_25 の方法)。
inline u32 run(u32 r, u32 d, u64 n) {
 using namespace se1;
 if(n == 0) return 0;
 if(r == 0) return d == 0;
 const int D= int(d);
 const auto t= pow_table(D, d);
 if(r == 1) {
  std::vector<u32> F(D + 2);
  for(int k= 0; k <= D; ++k) F[k + 1]= add(F[k], t[k]);
  return lagrange_at(F, n, binom_row(D + 1));
 }
 const auto S= prefix_sums(t, r);
 if(n <= u64(D) + 1) return S[n];
 const auto bn= binom_row(D);
 std::vector<u32> g;
 const u32 c= const_term(S, r, bn, &g);
 return add(c, mul(pw(r, n), lagrange_at(g, n, bn)));
}
