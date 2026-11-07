#pragma once
// NeoLibrary 95f83f0 の neo/linalg/Matrix.hpp の Frobenius 標準形の道 (Frob と zmat::pow) の写しで、x64 の GCC でだけ lib-neo が
// zmod_frob の 1.1〜1.2 倍かかる件を調べるためのもの。FIX = false はそのままの写し、FIX = true は dot の最後で 4 本の和を Z の
// 足し算 (2M 以上なら引く) と val() でまとめる代わりに、u64 のまま 2 回畳んで足し、剰余を 1 回だけ取る形にしたもの。x64 の
// GCC 15 は Z の足し算と val() の比べる処理を分岐にしていて (-S で gen の中に 4 か所)、値が乱数に近いので予測を外す。
// zmod_frob の dot は u64 の和を % M で 1 回還元するので分岐がない。決めたら消す。
#include "../common.hpp"
#include "neo/linalg/Matrix.hpp"
namespace neo_frob_ab {
using namespace linalg_internal::zmat;
template <class Z, bool FIX> struct Frob {
 static u32 rd(u64 x) { return Z(x).val(); }
 static u64 fold1(u64 x, u64 r32) { return (x >> 32) * r32 + u32(x); }
 // sum_i x[i] y[i] mod M (x[i], y[i] < M)。
 static u32 dot(const u32* x, const u32* y, int n, u64 r32) {
  const __m256i Rv= _mm256_set1_epi64x(r32), LO= _mm256_set1_epi64x(0xffffffffll);
  __m256i acc= _mm256_setzero_si256();
  int i= 0, t= 0;
  for(; i + 4 <= n; i+= 4) {
   acc= _mm256_add_epi64(acc, _mm256_mul_epu32(_mm256_cvtepu32_epi64(_mm_loadu_si128((const __m128i*)(x + i))), _mm256_cvtepu32_epi64(_mm_loadu_si128((const __m128i*)(y + i)))));
   if(++t == 8) acc= _mm256_add_epi64(_mm256_mul_epu32(_mm256_srli_epi64(acc, 32), Rv), _mm256_and_si256(acc, LO)), t= 0;
  }
  alignas(32) u64 s[4];
  if constexpr(FIX) {
   // 和を 2 回畳み (各 2^60 + 2^32 未満)、4 本と端の分を u64 のまま足して剰余を 1 回取る。
   acc= _mm256_add_epi64(_mm256_mul_epu32(_mm256_srli_epi64(acc, 32), Rv), _mm256_and_si256(acc, LO));
   acc= _mm256_add_epi64(_mm256_mul_epu32(_mm256_srli_epi64(acc, 32), Rv), _mm256_and_si256(acc, LO));
   _mm256_store_si256((__m256i*)s, acc);
   u64 q= s[0] + s[1] + s[2] + s[3];
   for(; i < n; ++i) q+= u64(x[i]) * y[i];
   return rd(q);
  } else {
   _mm256_store_si256((__m256i*)s, acc);
   Z r= Z(s[0]) + Z(s[1]) + Z(s[2]) + Z(s[3]);
   u64 q= 0;
   for(; i < n; ++i) q+= u64(x[i]) * y[i];
   return (r + Z(q)).val();
  }
 }
 // t[0..len) (畳んだ u64) を、f (monic、次数 d) で割った余りに直して r[0..d) (M 未満) に書く。nf[j] = -f[j] mod M。
 static void reduce(std::vector<u64>& t, int len, const std::vector<u32>& nf, int d, std::vector<u32>& r, u64 r32) {
  int cnt= 0;
  for(int k= len - 1; k >= d; --k) {
   const u32 c= rd(t[k]);
   if(c) {
    u64* q= &t[k - d];
    for(int j= 0; j < d; ++j) q[j]+= u64(c) * nf[j];
   }
   if(++cnt == 8) {
    for(int j= std::max(0, k - d); j < k; ++j) t[j]= fold1(t[j], r32);
    cnt= 0;
   }
  }
  for(int j= 0; j < d; ++j) r[j]= rd(t[j]);
 }
 // x^K mod f (K >= 1、f は monic で次数 d)。上の bit から 2 乗と x 倍を重ねる。
 static std::vector<u32> xpow(u64 K, const std::vector<u32>& f, int d, u64 r32) {
  const u32 M= Z::mod();
  std::vector<u32> nf(d), r(d);
  for(int j= 0; j < d; ++j) nf[j]= f[j] ? M - f[j] : 0;
  std::vector<u64> t(2 * d);
  r[0]= 1 % M;
  for(int b= 63 - std::countl_zero(K); b >= 0; --b) {
   std::fill(t.begin(), t.end(), 0);
   int cnt= 0;
   for(int i= 0; i < d; ++i) {
    if(!r[i]) continue;
    const u64 a= r[i];
    u64* q= &t[i];
    for(int j= 0; j < d; ++j) q[j]+= a * r[j];
    if(++cnt == 8) {
     for(auto& v: t) v= fold1(v, r32);
     cnt= 0;
    }
   }
   for(auto& v: t) v= fold1(v, r32);
   reduce(t, 2 * d - 1, nf, d, r, r32);
   if(K >> b & 1) {  // r = x r mod f
    const u64 c= r[d - 1];
    for(int j= d - 1; j >= 1; --j) r[j]= rd(r[j - 1] + c * nf[j]);
    r[0]= rd(c * nf[0]);
   }
  }
  return r;
 }
 static u64 rng() {
  static u64 s= u64(std::chrono::steady_clock::now().time_since_epoch().count());
  u64 z= (s+= 0x9e3779b97f4a7c15ull);
  z= (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9ull, z= (z ^ (z >> 27)) * 0x94d049bb133111ebull;
  return z ^ (z >> 31);
 }
 // 乱数のベクトル v から Krylov の鎖 v, Av, A^2 v, ... を作り、それまでに集めたベクトルの張る空間に入ったところで止める。
 // 止まったときの関係 f(A) v = w (w は前の鎖の張る空間の元) から鎖の多項式 f が分かる。w が 0 でなければ、前の鎖ごとに w の
 // 成分 g_c を f で割り (乱数がうまく選べていれば割り切れる)、u = sum_c (g_c / f)(A) v_c を引いた v - u から鎖を作り直す。
 // 全体を鎖に分けると、鎖を並べた基底 T で A = T F T^{-1} (F は同伴行列を並べたブロック対角) になり、A^K = T F^K T^{-1}。
 // F^K のブロックの列 j は x^{K+j} mod f。鎖のベクトルは、要の位置で正規化した掃き出しの行 [e | α] (e = sum_j α_j t_j) として
 // 持ち、新しいベクトルを入れるたびに前の行で消す。T^{-1} の列 piv_k は、e の部分を後ろから単位ベクトルに直したときの α の
 // 部分。やり直しても割り切れなければ false を返す。a、out は n × n (M 未満)。
 static bool pow(int n, u64 K, const u32* a, u32* out) {
  const u32 M= Z::mod();
  const u64 R32= (u64(1) << 32) % M;
  // 行は [e (n 個) | α (n + 1 個)]。α の最後は、空間が埋まったあとの関係の分。
  const int W= 2 * n + 1;
  std::vector<u32> Y(std::size_t(n) * W), T(std::size_t(n) * n);  // T の鎖のベクトル t_j は T[j n .. j n + n)
  std::vector<int> piv(n), len(n);
  std::vector<u64> y(W);
  int r= 0;
  // x (= t_j) を入れる。新しい行になれば true。前の行の張る空間に入れば false で、y[n..n+j] に関係 (y[n+j] = 1) が残る。
  auto insert= [&](const u32* x, int j) -> bool {
   for(int i= 0; i < n; ++i) y[i]= x[i];
   std::fill(y.begin() + n, y.begin() + n + j + 1, 0);
   y[n + j]= 1;
   int cnt= 0;
   for(int k= 0; k < r; ++k) {
    const u32 c= rd(y[piv[k]]);
    if(!c) continue;
    axpy(y.data(), &Y[std::size_t(k) * W], M - c, len[k]);
    if(++cnt == 8) fold_all(y.data(), n + j + 1, R32), cnt= 0;
   }
   int p= -1;
   for(int i= 0; i < n + j + 1; ++i) {
    y[i]= rd(y[i]);
    if(p < 0 && i < n && y[i]) p= i;
   }
   if(p < 0) return false;
   const u64 iv= Z::raw(u32(y[p])).inv().val();
   u32* row= &Y[std::size_t(r) * W];
   for(int i= 0; i < n + j + 1; ++i) row[i]= rd(y[i] * iv);
   std::fill(row + n + j + 1, row + W, 0);
   piv[r]= p, len[r]= n + j + 1, ++r;
   return true;
  };
  std::vector<u32> cur(n), nxt(n);
  // v から鎖を作る (s 番目から)。前の空間に入ったときの鎖の長さ d を返し、y に関係を残す。
  auto gen= [&](const std::vector<u32>& v, int s) -> int {
   r= s;
   std::copy(v.begin(), v.end(), cur.begin());
   for(int j= s;; ++j) {
    if(!insert(cur.data(), j)) return j - s;
    std::copy(cur.begin(), cur.end(), &T[std::size_t(j) * n]);
    for(int i= 0; i < n; ++i) nxt[i]= dot(a + std::size_t(i) * n, cur.data(), n, R32);
    std::swap(cur, nxt);
   }
  };
  struct Block {
   int s, d;
   std::vector<u32> f;  // f[0..d]、f[d] = 1
  };
  std::vector<Block> blocks;
  std::vector<u32> v(n), w(n);
  std::vector<u64> acc(n);
  // s 番目からの鎖を 1 本作る。乱数のベクトルを TB 本試して鎖の最も長いものを選ぶ。前の鎖が A の最小多項式の次数の大きい順に
  // 選べていれば、割り算は割り切れる。法が小さいと乱数のベクトルが最大の次数を外しやすいので、TB を増やす。
  const int TB= M < 1024 ? 4 : 1;
  auto build= [&](int s) -> bool {
   int d= -1;
   bool last= false;
   for(int t= 0; t < TB && d < n - s; ++t) {
    for(auto& x: w) x= rd(rng());
    const int e= gen(w, s);
    if((last= e > d)) d= e, std::swap(v, w);
   }
   if(!last) d= gen(v, s);
   if(d == 0) return false;
   bool clean= true;
   for(int j= 0; j < s && clean; ++j) clean= y[n + j] == 0;
   if(!clean) {  // 前の鎖ごとに g_c = -(関係の成分) を f で割り、u = sum_c h_c(A) v_c を v から引く
    std::vector<u32> f(d + 1);
    for(int i= 0; i <= d; ++i) f[i]= u32(y[n + s + i]);
    std::fill(acc.begin(), acc.end(), 0);
    int cnt= 0;
    for(const auto& B: blocks) {
     std::vector<u64> g(B.d);
     for(int i= 0; i < B.d; ++i) g[i]= y[n + B.s + i] ? M - y[n + B.s + i] : 0;
     for(int deg= B.d - 1; deg >= d; --deg) {  // g -= q x^{deg - d} f
      const u32 q= rd(g[deg]);
      g[deg]= 0;
      if(!q) continue;
      for(int i= 0; i < d; ++i) g[deg - d + i]= rd(g[deg - d + i] + u64(q) * (f[i] ? M - f[i] : 0));
      axpy(acc.data(), &T[std::size_t(B.s + deg - d) * n], q, n);
      if(++cnt == 8) fold_all(acc.data(), n, R32), cnt= 0;
     }
     for(int i= 0; i < std::min(d, B.d); ++i)
      if(rd(g[i])) return false;
    }
    for(int i= 0; i < n; ++i) {
     const u32 t= rd(acc[i]);
     v[i]= v[i] >= t ? v[i] - t : v[i] + M - t;
    }
    d= gen(v, s);
    if(d == 0) return false;
    for(int j= 0; j < s; ++j)
     if(y[n + j]) return false;
   }
   std::vector<u32> f(d + 1);
   for(int i= 0; i <= d; ++i) f[i]= u32(y[n + s + i]);
   blocks.push_back({s, d, f});
   return true;
  };
  // 割り切れなければ、前の鎖の選び方が悪かったとみて最初からやり直す (法が小さければ 8 回、大きければ 2 回まで)。
  for(int g= 0;; ++g) {
   blocks.clear(), r= 0;
   bool ok= true;
   while(r < n && (ok= build(r)));
   if(ok) break;
   if(g + 1 == (M < 1024 ? 8 : 2)) return false;
  }
  // T^{-1}: α'_k = α_k - sum_{kk > k} e_k[piv_kk] α'_kk を k の大きい順に。T^{-1} の列 piv_k が α'_k。
  std::vector<u32> Ap(std::size_t(n) * n);
  for(int k= n - 1; k >= 0; --k) {
   const u32* row= &Y[std::size_t(k) * W];
   for(int j= 0; j < n; ++j) acc[j]= row[n + j];
   int cnt= 0;
   for(int kk= k + 1; kk < n; ++kk) {
    const u32 c= row[piv[kk]];
    if(!c) continue;
    axpy(acc.data(), &Ap[std::size_t(kk) * n], M - c, n);
    if(++cnt == 8) fold_all(acc.data(), n, R32), cnt= 0;
   }
   for(int j= 0; j < n; ++j) Ap[std::size_t(k) * n + j]= rd(acc[j]);
  }
  std::vector<u32> Tm(std::size_t(n) * n), Fk(std::size_t(n) * n), Ti(std::size_t(n) * n), P(std::size_t(n) * n);
  for(int j= 0; j < n; ++j)
   for(int i= 0; i < n; ++i) Tm[std::size_t(i) * n + j]= T[std::size_t(j) * n + i];
  for(int k= 0; k < n; ++k)
   for(int j= 0; j < n; ++j) Ti[std::size_t(j) * n + piv[k]]= Ap[std::size_t(k) * n + j];
  for(const auto& B: blocks) {
   std::vector<u32> col= xpow(K, B.f, B.d, R32), nf(B.d);
   for(int i= 0; i < B.d; ++i) nf[i]= B.f[i] ? M - B.f[i] : 0;
   for(int jj= 0; jj < B.d; ++jj) {
    for(int ii= 0; ii < B.d; ++ii) Fk[std::size_t(B.s + ii) * n + B.s + jj]= col[ii];
    const u64 c= col[B.d - 1];  // col = x col mod f
    for(int ii= B.d - 1; ii >= 1; --ii) col[ii]= rd(col[ii - 1] + c * nf[ii]);
    col[0]= rd(c * nf[0]);
   }
  }
  const Mod md(M);
  tile_mul<Z>(Tm.data(), Fk.data(), P.data(), n, n, n, md);
  tile_mul<Z>(P.data(), Ti.data(), out, n, n, n, md);
  return true;
 }
};
template <class Z, bool FIX> void pow(const Z* A, int n, u64 K, Z* R) {
 if(n == 0) return;
 if(K == 0) {
  for(int i= 0; i < n; ++i)
   for(int j= 0; j < n; ++j) R[std::size_t(i) * n + j]= Z(i == j);
  return;
 }
 const std::size_t nn= std::size_t(n) * n;
 if(n < 8) {
  std::vector<Z> b(A, A + nn), r(A, A + nn), t(nn);
  for(int i= 63 - std::countl_zero(K) - 1; i >= 0; --i) {
   mul(r.data(), r.data(), t.data(), n, n, n), std::swap(r, t);
   if(K >> i & 1) mul(r.data(), b.data(), t.data(), n, n, n), std::swap(r, t);
  }
  std::copy(r.begin(), r.end(), R);
  return;
 }
 std::vector<u32> a(nn);
 for(std::size_t i= 0; i < nn; ++i) a[i]= A[i].val();
 if(K >= 2 && prime_mod<Z>() && frob_worth(n, K)) {
  std::vector<u32> o(nn);
  if(Frob<Z, FIX>::pow(n, K, a.data(), o.data())) {
   for(std::size_t i= 0; i < nn; ++i) R[i]= Z::raw(o[i]);
   return;
  }
 }
 // 二分累乗。上の bit から 2 乗と A 倍を重ねる。法が奇数なら、値を Montgomery の表現 (x 2^32 mod M) で持って掛け続ける。
 const Mod md(Z::mod());
 const int w= (n + 7) & ~7;
 const std::size_t ww= std::size_t(w) * w;
 std::vector<u32> r(ww), t(ww), bp(ww + 8), ap(ww + 8);
 for(int i= 0; i < n; ++i)
  for(int j= 0; j < n; ++j) r[std::size_t(i) * w + j]= md.odd ? md.to_mont(a[std::size_t(i) * n + j]) : a[std::size_t(i) * n + j];
 pack_bands(r.data(), n, n, w, ap.data(), w);
 for(int i= 63 - std::countl_zero(K) - 1; i >= 0; --i) {
  pack_bands(r.data(), n, n, w, bp.data(), w);
  if(md.odd) tile<2, Z>(r.data(), bp.data(), n, w, n, t.data(), w, md);
  else tile<0, Z>(r.data(), bp.data(), n, w, n, t.data(), w, md);
  std::swap(r, t);
  if(K >> i & 1) {
   if(md.odd) tile<2, Z>(r.data(), ap.data(), n, w, n, t.data(), w, md);
   else tile<0, Z>(r.data(), ap.data(), n, w, n, t.data(), w, md);
   std::swap(r, t);
  }
 }
 for(int i= 0; i < n; ++i)
  for(int j= 0; j < n; ++j) R[std::size_t(i) * n + j]= Z::raw(md.odd ? md.redc(r[std::size_t(i) * w + j]) : r[std::size_t(i) * w + j]);
}
}  // namespace neo_frob_ab
