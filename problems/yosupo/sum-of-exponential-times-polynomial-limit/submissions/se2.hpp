#pragma once
// Σ_{i≥0} r^i i^d (se2)。se1 と同じ min_25 の方法で、10^7 要素の作業を配列 2 本 (huge page) と AVX2 の走査にまとめたもの。
// 1. i^d の表: 区画ごとの篩で合成数 m に m/p と p^d を記録して t[m] = t[m/p] p^d とし、素数の冪は 8 個ずつ AVX2 の
//    Montgomery の積で求める (4 本を並べて待ちを隠す)。2 番目以降の区画では m/p が前の区画にあるので、素数は区画の終わりに
//    まとめて求めてよい。
// 2. S(k) = Σ_{i<k} r^i t_i: r^i を 8 レーンの鎖で作り、ベクトルの中の累積和と前のベクトルからの繰り上げで、t の上に置く。
// 3. c = Σ_k u_k S(k) / (1 - r)^{d+1}、u_k = C(d+1, k) (-r)^{d+1-k} (の符号つき): 比 u_k / u_{k-1} = -(d+2-k) / (k r) の
//    逆元を 8 本の鎖でまとめて求め、比の累積積を区間 4 つの鎖で作りながら S と掛けて和にためる (累積積は配列に置かない)。
// 4. g(n) = Σ_k (S(k) - c) r^{-k} L_k(x) (x = n mod M、L_k はラグランジュの基底): 比 W_{k+1} / W_k
//    = -(d-k)(x-k) / ((k+1)(x-k-1) r) (W_k = L_k r^{-k}) を 3. と同じ形で求め、Σ S_k W_k と Σ W_k をためる。
#ifdef __x86_64__
#include <immintrin.h>
#else
#include <simde/x86/avx2.h>
#endif
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <new>
#include <vector>
#ifdef __linux__
#include <sys/mman.h>
#endif
namespace se2 {
constexpr u32 M= 998244353;
constexpr u32 MI= [] {  // -M^{-1} mod 2^32
 u32 x= M;
 for(int i= 0; i < 5; ++i) x*= 2 - M * x;
 return -x;
}();
constexpr u32 R1= u32((u64(1) << 32) % M);  // R mod M (Montgomery の 1)
constexpr u32 R2= u32(u64(R1) * R1 % M);
constexpr u32 R3= u32(u64(R2) * R1 % M);
inline u32 mul(u32 a, u32 b) { return u32(u64(a) * b % M); }
inline u32 add(u32 a, u32 b) { return a + b >= M ? a + b - M : a + b; }
inline u32 sub(u32 a, u32 b) { return a >= b ? a - b : a + M - b; }
inline u32 pw(u32 a, u64 e) {
 u32 r= 1;
 for(; e; e>>= 1, a= mul(a, a))
  if(e & 1) r= mul(r, a);
 return r;
}
inline u32 inv(u32 a) { return pw(a, M - 2); }
template <class T> struct HugeAlloc {
 using value_type= T;
 static constexpr std::size_t H= std::size_t(1) << 21, P= 4096;
 HugeAlloc()= default;
 template <class U> HugeAlloc(const HugeAlloc<U>&) {}
 T* allocate(std::size_t n) {
#ifdef __linux__
  if(n * sizeof(T) >= H) {
   const std::size_t bytes= (n * sizeof(T) + P - 1) & ~(P - 1);
   void* m= mmap(nullptr, bytes + H, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
   if(m == MAP_FAILED) throw std::bad_alloc();
   char *p= static_cast<char*>(m), *a= reinterpret_cast<char*>((reinterpret_cast<std::uintptr_t>(p) + H - 1) & ~(H - 1));
   if(a != p) munmap(p, a - p);
   munmap(a + bytes, p + H - a);
   madvise(a, bytes, MADV_HUGEPAGE);
   return reinterpret_cast<T*>(a);
  }
#endif
  return std::allocator<T>().allocate(n);
 }
 void deallocate(T* p, std::size_t n) {
#ifdef __linux__
  if(n * sizeof(T) >= H) {
   munmap(p, (n * sizeof(T) + P - 1) & ~(P - 1));
   return;
  }
#endif
  std::allocator<T>().deallocate(p, n);
 }
 template <class U> bool operator==(const HugeAlloc<U>&) const { return true; }
 template <class U> bool operator!=(const HugeAlloc<U>&) const { return false; }
};
using hvec= std::vector<u32, HugeAlloc<u32>>;
// x y 2^{-32} mod M を 8 個ずつ (x、y が 2M 未満なら結果も 2M 未満)。
[[gnu::always_inline]] inline __m256i mont8(__m256i x, __m256i y) {
 const __m256i mv= _mm256_set1_epi32(M), mi= _mm256_set1_epi32(MI);
 const __m256i te= _mm256_mul_epu32(x, y), to= _mm256_mul_epu32(_mm256_srli_epi64(x, 32), _mm256_srli_epi64(y, 32));
 const __m256i ue= _mm256_add_epi64(te, _mm256_mul_epu32(_mm256_mul_epu32(te, mi), mv));
 const __m256i uo= _mm256_add_epi64(to, _mm256_mul_epu32(_mm256_mul_epu32(to, mi), mv));
 return _mm256_blend_epi32(_mm256_srli_epi64(ue, 32), uo, 0b10101010);
}
[[gnu::always_inline]] inline __m256i red8(__m256i x) { return _mm256_min_epu32(x, _mm256_sub_epi32(x, _mm256_set1_epi32(M))); }
[[gnu::always_inline]] inline __m256i addm8(__m256i x, __m256i y) { return red8(_mm256_add_epi32(x, y)); }  // M 未満どうし
// 8 個の素数 (M 未満) の d 乗 (M 未満で返す)。4 本を並べる。
inline void pow32(const u32* x, u32* out, u64 d) {
 __m256i b[4], r[4];
 const __m256i r2= _mm256_set1_epi32(R2), one= _mm256_set1_epi32(R1);
 for(int q= 0; q < 4; ++q) b[q]= mont8(_mm256_loadu_si256((const __m256i*)(x + 8 * q)), r2), r[q]= one;
 int top= 63;
 while(top >= 0 && !(d >> top & 1)) --top;
 for(int i= top; i >= 0; --i) {
  for(int q= 0; q < 4; ++q) r[q]= mont8(r[q], r[q]);
  if(d >> i & 1)
   for(int q= 0; q < 4; ++q) r[q]= mont8(r[q], b[q]);
 }
 for(int q= 0; q < 4; ++q) _mm256_storeu_si256((__m256i*)(out + 8 * q), red8(mont8(r[q], _mm256_set1_epi32(1))));
}
// t[i] = i^d (0 ≤ i ≤ D)。t は D + 1 + 64 個以上。
inline void pow_table(u32* t, int D, u64 d) {
 if(d == 0) return std::fill(t, t + D + 1, 1u);
 t[0]= 0;
 if(D >= 1) t[1]= 1;
 int sq= int(std::sqrt(double(D)));
 while((long long)sq * sq > D) --sq;
 while((long long)(sq + 1) * (sq + 1) <= D) ++sq;
 std::vector<char> comp(sq + 1);
 std::vector<int> ps;
 std::vector<u32> pp;
 for(int i= 2; i <= sq; ++i)
  if(!comp[i]) {
   ps.push_back(i), pp.push_back(pw(i, d));
   for(int j= i * i; j <= sq; j+= i) comp[j]= 1;
  }
 constexpr int B= 1 << 15;
 std::vector<u32> cof(B), ppv(B), pr(B + 32), prv(B + 32);
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
  if(L == 2) {  // 最初の区画は m/p が区画の中にあるので、素数もその場で求める
   for(int m= L; m < R; ++m) {
    const u32 c= cof[m - L];
    t[m]= c ? mul(t[c], ppv[m - L]) : pw(u32(m), d);
   }
   continue;
  }
  int np= 0;
  for(int m= L; m < R; ++m) {
   const u32 c= cof[m - L];
   if(c) t[m]= mul(t[c], ppv[m - L]);
   else pr[np++]= u32(m);
  }
  for(int i= np; i % 32; ++i) pr[i]= 1;
  for(int i= 0; i < np; i+= 32) pow32(pr.data() + i, prv.data() + i, d);
  for(int i= 0; i < np; ++i) t[pr[i]]= prv[i];
 }
}
// S(k) = Σ_{i<k} r^i t_i を t[k] に置き (0 ≤ k ≤ D)、S(D+1) を返す。
inline u32 prefix_sums(u32* t, int D, u32 r) {
 const int n= D + 1;
 const int nv= n / 8;
 // rv = r^{8j+b} R (Montgomery)、r8 = r^8 R
 alignas(32) u32 a[8];
 {
  u32 x= 1;
  for(int b= 0; b < 8; ++b) a[b]= mul(x, R1), x= mul(x, r);
 }
 __m256i rv= _mm256_load_si256((const __m256i*)a);
 const __m256i r8= _mm256_set1_epi32(mul(pw(r, 8), R1));
 const __m256i i1= _mm256_setr_epi32(0, 0, 1, 2, 3, 4, 5, 6), i2= _mm256_setr_epi32(0, 0, 0, 1, 2, 3, 4, 5), i4= _mm256_setr_epi32(0, 0, 0, 0, 0, 1, 2, 3);
 const __m256i m1= _mm256_setr_epi32(0, -1, -1, -1, -1, -1, -1, -1), m2= _mm256_setr_epi32(0, 0, -1, -1, -1, -1, -1, -1), m4= _mm256_setr_epi32(0, 0, 0, 0, -1, -1, -1, -1);
 const __m256i i7= _mm256_set1_epi32(7);
 __m256i carry= _mm256_setzero_si256();
 for(int j= 0; j < nv; ++j) {
  const __m256i w= red8(mont8(_mm256_loadu_si256((const __m256i*)(t + 8 * j)), rv));  // r^k t_k
  rv= mont8(rv, r8);
  __m256i s= addm8(w, _mm256_and_si256(_mm256_permutevar8x32_epi32(w, i1), m1));
  s= addm8(s, _mm256_and_si256(_mm256_permutevar8x32_epi32(s, i2), m2));
  s= addm8(s, _mm256_and_si256(_mm256_permutevar8x32_epi32(s, i4), m4));
  const __m256i incl= addm8(s, carry);
  // 排他的な累積和 = 含む累積和 - w
  const __m256i excl= red8(_mm256_add_epi32(_mm256_sub_epi32(incl, w), _mm256_set1_epi32(M)));
  _mm256_storeu_si256((__m256i*)(t + 8 * j), excl);
  carry= _mm256_permutevar8x32_epi32(incl, i7);
 }
 _mm256_store_si256((__m256i*)a, carry);
 u32 S= a[0];
 _mm256_store_si256((__m256i*)a, rv);
 u32 rp= mul(a[0] % M, inv(R1));  // r^{8 nv} (R を外す)
 for(int k= 8 * nv; k < n; ++k) {
  const u32 x= t[k];
  t[k]= S;
  S= add(S, mul(rp, x)), rp= mul(rp, r);
 }
 return S;
}
// gen(j) が返す 8 個 (1 以上 M 未満) の逆元に σ を掛けたものを、後ろ向きに back(j, σ/値) で渡す。全部の値の積を返す。
template <class Gen, class Back> u32 batch_inv(int steps, u32* W, u32 sigma, Gen gen, Back back) {
 if(!steps) return 1;
 __m256i acc= _mm256_set1_epi32(1);
 for(int j= 0; j < steps; ++j) acc= mont8(acc, gen(j)), _mm256_storeu_si256((__m256i*)(W + 8 * j), acc);
 alignas(32) u32 c[8];
 _mm256_store_si256((__m256i*)c, acc);
 u32 prod= 1;
 for(int b= 0; b < 8; ++b) c[b]%= M, prod= mul(prod, c[b]), c[b]= mul(inv(c[b]), sigma);
 prod= mul(prod, pw(R1, u64(8) * steps));
 __m256i cur= _mm256_load_si256((const __m256i*)c);
 for(int j= steps - 1; j >= 0; --j) {
  const __m256i pre= j ? _mm256_loadu_si256((const __m256i*)(W + 8 * (j - 1))) : _mm256_set1_epi32(1);
  back(j, red8(mont8(cur, pre)));
  cur= mont8(cur, gen(j));
 }
 return prod;
}
// 比 rho[i] (Montgomery の形、i < N、N 以降は R1 で埋める) の累積積 P_i = Π_{j≤i} rho_j を区間 4 つの鎖で作り、
// A = Σ_i val[i] P_i と B = Σ_i P_i を返す (どちらも本当の値)。val は M 未満 (N 以降は読むが足さない)。
inline void scan_acc(const u32* rho, const u32* val, int N, u32& A, u32& B) {
 constexpr int SG= 4;
 const int nv= (N + 7) / 8, per= (nv + SG - 1) / SG;  // 区間ごとのベクトル数
 const __m256i one= _mm256_set1_epi32(R1), lo32= _mm256_set1_epi64x(0xffffffffll), lane= _mm256_setr_epi32(0, 1, 2, 3, 4, 5, 6, 7), nn= _mm256_set1_epi32(N);
 const __m256i i1= _mm256_setr_epi32(0, 0, 1, 2, 3, 4, 5, 6), i2= _mm256_setr_epi32(0, 0, 0, 1, 2, 3, 4, 5), i4= _mm256_setr_epi32(0, 0, 0, 0, 0, 1, 2, 3), i7= _mm256_set1_epi32(7);
 __m256i carry[SG], ae[SG], ao[SG], be[SG], bo[SG];
 for(int s= 0; s < SG; ++s) carry[s]= one, ae[s]= ao[s]= be[s]= bo[s]= _mm256_setzero_si256();
 auto step= [&](int s, int j) {
  __m256i x= _mm256_loadu_si256((const __m256i*)(rho + 8 * j));
  x= mont8(x, _mm256_blend_epi32(_mm256_permutevar8x32_epi32(x, i1), one, 0b00000001));
  x= mont8(x, _mm256_blend_epi32(_mm256_permutevar8x32_epi32(x, i2), one, 0b00000011));
  x= mont8(x, _mm256_blend_epi32(_mm256_permutevar8x32_epi32(x, i4), one, 0b00001111));
  const __m256i p= mont8(x, carry[s]);  // 累積積 (Montgomery、2M 未満)
  carry[s]= _mm256_permutevar8x32_epi32(p, i7);
  const __m256i ok= _mm256_cmpgt_epi32(nn, _mm256_add_epi32(lane, _mm256_set1_epi32(8 * j)));          // N 以降のレーンは足さない
  const __m256i v= _mm256_and_si256(mont8(_mm256_loadu_si256((const __m256i*)(val + 8 * j)), p), ok);  // val P (本当の値、2M 未満)
  const __m256i pp= _mm256_and_si256(mont8(p, _mm256_set1_epi32(1)), ok);                              // P (本当の値)
  ae[s]= _mm256_add_epi64(ae[s], _mm256_and_si256(v, lo32)), ao[s]= _mm256_add_epi64(ao[s], _mm256_srli_epi64(v, 32));
  be[s]= _mm256_add_epi64(be[s], _mm256_and_si256(pp, lo32)), bo[s]= _mm256_add_epi64(bo[s], _mm256_srli_epi64(pp, 32));
 };
 for(int t= 0; t < per; ++t)
  for(int s= 0; s < SG; ++s) {
   const int j= s * per + t;
   if(j < nv) step(s, j);
  }
 // 区間 s の始まりの値 = 前の区間の総積の積
 u32 start= 1;
 u64 sa= 0, sb= 0;
 for(int s= 0; s < SG; ++s) {
  if(s * per >= nv) break;
  alignas(32) u64 x[4], y[4];
  _mm256_store_si256((__m256i*)x, _mm256_add_epi64(ae[s], ao[s]));
  _mm256_store_si256((__m256i*)y, _mm256_add_epi64(be[s], bo[s]));
  const u32 pa= u32((x[0] % M + x[1] % M + x[2] % M + x[3] % M) % M), pb= u32((y[0] % M + y[1] % M + y[2] % M + y[3] % M) % M);
  sa+= mul(start, pa), sb+= mul(start, pb);
  alignas(32) u32 c[8];
  _mm256_store_si256((__m256i*)c, carry[s]);
  start= mul(start, mul(c[0] % M, inv(R1)));  // 区間の総積 (Montgomery の R を外す)
 }
 A= u32(sa % M), B= u32(sb % M);
}
}  // namespace se2
// Σ_{i≥0} r^i i^d。S(x) = c + r^x g(x) の c。
inline u32 run(u32 r, u32 d) {
 using namespace se2;
 if(r == 0) return d == 0;
 const int D= int(d);
 const int cap= 8 * ((D + 2 + 7) / 8) + 64;
 hvec t(cap, 0), W(cap, 0), rho(cap, R1);
 pow_table(t.data(), D, d);
 t[D + 1]= prefix_sums(t.data(), D, r);
 const int N= D + 1, steps= (N + 7) / 8;
 const __m256i lane= _mm256_setr_epi32(0, 1, 2, 3, 4, 5, 6, 7), rR= _mm256_set1_epi32(mul(r, R1));
 auto gen= [&](int s) {  // j r (j = 8s + b + 1 ≤ N、それ以外は 1)
  const __m256i j= _mm256_add_epi32(lane, _mm256_set1_epi32(8 * s + 1));
  return _mm256_blendv_epi8(red8(mont8(j, rR)), _mm256_set1_epi32(1), _mm256_cmpgt_epi32(j, _mm256_set1_epi32(N)));
 };
 batch_inv(steps, W.data(), R3, gen, [&](int s, __m256i iv) {
  const __m256i j= _mm256_add_epi32(lane, _mm256_set1_epi32(8 * s + 1));
  const __m256i z= mont8(_mm256_sub_epi32(_mm256_set1_epi32(D + 2), j), _mm256_set1_epi32(1));  // (D+2-j) / R
  __m256i v= red8(mont8(iv, z));
  v= red8(_mm256_sub_epi32(_mm256_set1_epi32(M), v));
  v= _mm256_blendv_epi8(v, _mm256_set1_epi32(R1), _mm256_cmpgt_epi32(j, _mm256_set1_epi32(N)));
  _mm256_storeu_si256((__m256i*)(rho.data() + 8 * s), v);
 });
 u32 A, B;
 scan_acc(rho.data(), t.data() + 1, N, A, B);
 u32 u0= pw(r, D + 1);
 if((D + 1) & 1) u0= u0 ? M - u0 : 0;
 return mul(mul(u0, A), inv(pw(sub(1, r), D + 1)));
}
