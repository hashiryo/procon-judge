#pragma once
// A^K = r(A)、r = x^K mod χ (χ は A の特性多項式、Cayley–Hamilton の定理)。χ は _hess32.hpp の Hessenberg の形から求め、
// x^K mod χ は上の bit から多項式の 2 乗と x 倍を重ねて求める。和は u64 にため、8 回ごとに 2^32 の位を 2^32 mod M に
// 掛けて畳む。r(A) は Paterson–Stockmeyer の方法で戻す。A^0, ..., A^s を作り (積 s - 1 回)、r を x^s の多項式とみて
// Horner の方法で掛けていく (積 t - 1 回、t = ceil(N / s))。N = 200 なら積は 27 回で、二分累乗 (K ≈ 10^18 で 90 回ほど)
// より少ない。積は _tile.hpp の核で、行列の値は Montgomery の表現で持つ。
#include "_tile.hpp"
#include "_hess32.hpp"
namespace ps {
constexpr u64 R32= (u64(1) << 32) % MOD;
inline u64 fold(u64 x) { return (x >> 32) * R32 + u32(x); }
// t[0..len) (畳んだ u64) を、χ (monic、次数 d、χ[d] = 1) で割った余りに直して、r[0..d) (M 未満) に書く。
inline void reduce(vector<u64>& t, int len, const vector<u32>& nchi, int d, vector<u32>& r) {
 int cnt= 0;
 for(int k= len - 1; k >= d; --k) {
  const u32 c= u32(t[k] % MOD);
  if(c) {
   u64* q= &t[k - d];
   for(int j= 0; j < d; ++j) q[j]+= u64(c) * nchi[j];  // nchi[j] = M - χ[j] (mod M)
  }
  if(++cnt == 8) {
   for(int j= max(0, k - d); j < k; ++j) t[j]= fold(t[j]);
   cnt= 0;
  }
 }
 for(int j= 0; j < d; ++j) r[j]= u32(t[j] % MOD);
}
// x^K mod χ (K >= 1)。
inline vector<u32> xpow(u64 K, const vector<u32>& chi, int d) {
 vector<u32> nchi(d), r(d);
 for(int j= 0; j < d; ++j) nchi[j]= chi[j] ? MOD - chi[j] : 0;
 vector<u64> t(2 * d);
 r[0]= 1;
 for(int b= 63 - __builtin_clzll(K); b >= 0; --b) {
  // r = r^2 mod χ
  fill(t.begin(), t.end(), 0);
  int cnt= 0;
  for(int i= 0; i < d; ++i) {
   if(!r[i]) continue;
   const u64 a= r[i];
   u64* q= &t[i];
   for(int j= 0; j < d; ++j) q[j]+= a * r[j];
   if(++cnt == 8) {
    for(auto& v : t) v= fold(v);
    cnt= 0;
   }
  }
  for(auto& v : t) v= fold(v);
  reduce(t, 2 * d - 1, nchi, d, r);
  if(K >> b & 1) {  // r = x r mod χ
   const u64 c= r[d - 1];
   for(int j= d - 1; j >= 1; --j) r[j]= u32((r[j - 1] + c * nchi[j]) % MOD);
   r[0]= u32(c * nchi[0] % MOD);
  }
 }
 return r;
}
// out = (base + sum_i c[i] P[i]) mod M を w × w の各成分で求める (P[i] と base は Montgomery の表現、c[i] はそのままの値)。
// c x 2^32 = (c x) 2^32 なので、結果も Montgomery の表現になる。
inline void combine(const u32* base, const vector<const u32*>& P, const u32* c, int s, size_t sz, vector<u64>& acc, u32* out) {
 if(base)
  for(size_t e= 0; e < sz; ++e) acc[e]= base[e];
 else fill(acc.begin(), acc.begin() + sz, 0);
 int cnt= 0;
 for(int i= 0; i < s; ++i) {
  if(!c[i]) continue;
  const u64 ci= c[i];
  const u32* __restrict p= P[i];
  u64* __restrict a= acc.data();
  for(size_t e= 0; e < sz; ++e) a[e]+= ci * p[e];
  if(++cnt == 8) {
   for(size_t e= 0; e < sz; ++e) a[e]= fold(a[e]);
   cnt= 0;
  }
 }
 for(size_t e= 0; e < sz; ++e) out[e]= u32(acc[e] % MOD);
}
inline vector<u32> pow(int n, u64 K, const vector<u32>& a) {
 const int w= (n + 7) / 8 * 8;
 const size_t sz= (size_t)w * w;
 vector<u32> A= tile::load(n, w, a);
 if(K < 2) return tile::store(n, w, tile::pow(A, K, w));
 vector<vector<u32>> M2(n, vector<u32>(n));
 for(int i= 0; i < n; ++i)
  for(int j= 0; j < n; ++j) M2[i][j]= a[(size_t)i * n + j];
 const vector<u32> chi= hess32::charpoly(n, M2);  // chi[0..n]、chi[n] = 1
 const vector<u32> r= xpow(K, chi, n);            // 次数 n 未満
 // s を、積の回数 (s - 1) + (ceil(n / s) - 1) が最も少なくなるように選ぶ。
 int s= 1;
 for(int c= 1; c <= n; ++c)
  if((c - 1) + ((n + c - 1) / c - 1) < (s - 1) + ((n + s - 1) / s - 1)) s= c;
 const int t= (n + s - 1) / s;
 vector<vector<u32>> P(s + 1, vector<u32>(sz));
 for(int i= 0; i < w; ++i) P[0][(size_t)i * w + i]= tile::R1;
 P[1]= A;
 vector<u32> ap(sz), bp(sz);
 tile::pack(A.data(), ap.data(), w);
 for(int i= 2; i <= s; ++i) tile::mul_packed(P[i - 1].data(), ap.data(), P[i].data(), w);
 vector<const u32*> Pp(s);
 for(int i= 0; i < s; ++i) Pp[i]= P[i].data();
 vector<u64> acc(sz);
 vector<u32> cur(sz), tmp(sz), cf(s);
 auto coef= [&](int j) {  // r の x^{js} から x^{js+s-1} の係数
  for(int i= 0; i < s; ++i) cf[i]= j * s + i < n ? r[j * s + i] : 0;
  return cf.data();
 };
 combine(nullptr, Pp, coef(t - 1), s, sz, acc, cur.data());
 tile::pack(P[s].data(), bp.data(), w);
 for(int j= t - 2; j >= 0; --j) {
  tile::mul_packed(cur.data(), bp.data(), tmp.data(), w);
  combine(tmp.data(), Pp, coef(j), s, sz, acc, cur.data());
 }
 return tile::store(n, w, cur);
}
}  // namespace ps
inline vector<u32> run(int N, u64 K, const vector<u32>& a) { return ps::pow(N, K, a); }
