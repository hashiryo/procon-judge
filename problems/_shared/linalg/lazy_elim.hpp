#pragma once
// mod M (M < 2^30) の行列の掃き出しを、要を 8 本ずつまとめて行う核 (遅らせた更新)。
// yosupo の matrix-det、matrix-rank、system-of-linear-equations、inverse-matrix の提出が読む。
// 記録は algo-notes の notes/linear-algebra.md。
//
// 行は u64 で持つ。要を 1 本選ぶたびに残りの行を全部更新する代わりに、8 本の要の行 (M 未満に直して u64 に広げたもの) と、
// 行ごとの掛ける数 8 個を貯めておく。8 本たまったら、残りの各行を 1 回だけ読み書きして、8 本分の積をまとめて足す。
// 積は M^2 < 2^60 なので、2^62 + 2^32 未満の値に 8 本足しても u64 に収まる。足したあとは 2^32 の位を 2^32 mod M に掛けて
// 畳み、2^62 + 2^32 未満に戻す。要を探す列の値は、貯めている分をその場で足してから M 未満に直す。
#include <algorithm>
#include <vector>
#ifdef USE_SIMDE
#include <simde/x86/avx2.h>
#else
#include <immintrin.h>
#endif
namespace lazy_elim {
using u32= unsigned;
using u64= unsigned long long;
constexpr int K= 8;

// row[j] += sum_{s<8} f[s] * U[s][j] を j in [jb, w) で 4 個ずつ足してから畳む。U[s][j] < M、f[s] < M。
[[gnu::always_inline]] inline void update_row(u64* __restrict row, const u64* const* U, const u32* f, int jb, int w, u64 R) {
 const __m256i Rv= _mm256_set1_epi64x(R), LO= _mm256_set1_epi64x(0xffffffffll);
 const __m256i f0= _mm256_set1_epi64x(f[0]), f1= _mm256_set1_epi64x(f[1]), f2= _mm256_set1_epi64x(f[2]), f3= _mm256_set1_epi64x(f[3]);
 const __m256i f4= _mm256_set1_epi64x(f[4]), f5= _mm256_set1_epi64x(f[5]), f6= _mm256_set1_epi64x(f[6]), f7= _mm256_set1_epi64x(f[7]);
 const u64 *U0= U[0], *U1= U[1], *U2= U[2], *U3= U[3], *U4= U[4], *U5= U[5], *U6= U[6], *U7= U[7];
 for(int j= jb; j < w; j+= 4) {
  __m256i acc= _mm256_loadu_si256((const __m256i*)(row + j));
  acc= _mm256_add_epi64(acc, _mm256_mul_epu32(_mm256_loadu_si256((const __m256i*)(U0 + j)), f0));
  acc= _mm256_add_epi64(acc, _mm256_mul_epu32(_mm256_loadu_si256((const __m256i*)(U1 + j)), f1));
  acc= _mm256_add_epi64(acc, _mm256_mul_epu32(_mm256_loadu_si256((const __m256i*)(U2 + j)), f2));
  acc= _mm256_add_epi64(acc, _mm256_mul_epu32(_mm256_loadu_si256((const __m256i*)(U3 + j)), f3));
  acc= _mm256_add_epi64(acc, _mm256_mul_epu32(_mm256_loadu_si256((const __m256i*)(U4 + j)), f4));
  acc= _mm256_add_epi64(acc, _mm256_mul_epu32(_mm256_loadu_si256((const __m256i*)(U5 + j)), f5));
  acc= _mm256_add_epi64(acc, _mm256_mul_epu32(_mm256_loadu_si256((const __m256i*)(U6 + j)), f6));
  acc= _mm256_add_epi64(acc, _mm256_mul_epu32(_mm256_loadu_si256((const __m256i*)(U7 + j)), f7));
  acc= _mm256_add_epi64(_mm256_mul_epu32(_mm256_srli_epi64(acc, 32), Rv), _mm256_and_si256(acc, LO));
  _mm256_storeu_si256((__m256i*)(row + j), acc);
 }
}

// n 行 w 列 (w は 4 の倍数) の行列の前進消去。Z は neo/algebra/ZMod.hpp の ZMod。
// 要は列の順に探し、その列で値が 0 でない最初の行を選ぶ (残りの行の並びは、要を抜くたびに末尾の行で埋める)。
template <class Z> struct Elim {
 int n, w;
 std::vector<u64> a;  // n × w。2^62 + 2^32 未満で、その成分と mod M で等しい
 std::vector<int> rem;  // まだ要にしていない行
 std::vector<int> prow, pcol;  // 要にした行と列 (要にした順)
 std::vector<u32> pval;  // 要の値
 std::vector<u64> U;  // 要の行を M 未満に直したもの (要にした順に w 個ずつ)。要の列を 4 の倍数に切り下げた位置より左は 0 か使わない値
 Elim(int n_, int w_): n(n_), w(w_), a((size_t)n_ * w_), rem(n_) {
  for(int i= 0; i < n; ++i) rem[i]= i;
 }
 u64* row(int r) { return &a[(size_t)r * w]; }
 const u64* urow(int t) const { return &U[(size_t)t * w]; }
 u32 value(int r, int j) const { return Z(a[(size_t)r * w + j]).val(); }
 // 列 [0, lim) で要を探しながら前進消去する。要のない列があれば、stop なら false を返して止め、そうでなければ飛ばす。
 bool run(int lim, bool stop) {
  const u32 M= Z::mod();
  const u64 R= (u64(1) << 32) % M;
  U.assign((size_t)std::min(n, lim) * w, 0);
  std::vector<u32> F((size_t)n * K), v(n);
  std::vector<u64> zero(w);
  const u64* Ub[K];
  int col= 0;
  while(col < lim && !rem.empty()) {
   int bt= 0;
   for(; bt < K && col < lim && !rem.empty(); ++col) {
    const int nr= rem.size();
    int pi= -1;
    for(int i= 0; i < nr; ++i) {
     const int r= rem[i];
     const u32* fr= &F[(size_t)r * K];
     u64 x= a[(size_t)r * w + col];
     for(int s= 0; s < bt; ++s) x+= u64(fr[s]) * Ub[s][col];
     v[i]= Z(x).val();
     if(pi < 0 && v[i]) pi= i;
    }
    if(pi < 0) {
     if(stop) return false;
     continue;
    }
    const int p= rem[pi];
    u64* up= &U[prow.size() * w];
    const u32* fp= &F[(size_t)p * K];
    const u64* ap= row(p);
    for(int j= col & ~3; j < w; ++j) {
     u64 x= ap[j];
     for(int s= 0; s < bt; ++s) x+= u64(fp[s]) * Ub[s][j];
     up[j]= Z(x).val();
    }
    const Z iv= Z::raw(v[pi]).inv();
    for(int i= 0; i < nr; ++i) F[(size_t)rem[i] * K + bt]= (-(Z::raw(v[i]) * iv)).val();
    prow.push_back(p), pcol.push_back(col), pval.push_back(v[pi]);
    Ub[bt++]= up;
    rem[pi]= rem.back(), rem.pop_back();
   }
   if(!bt) break;
   for(int s= bt; s < K; ++s) Ub[s]= zero.data();
   const int jb= col & ~3;
   for(int r : rem) {
    u32* fr= &F[(size_t)r * K];
    for(int s= bt; s < K; ++s) fr[s]= 0;
    u32 any= 0;
    for(int s= 0; s < K; ++s) any|= fr[s];
    if(any) update_row(row(r), Ub, fr, jb, w, R);  // 掛ける数が全部 0 の行 (疎な行列で多い) は飛ばす
   }
  }
  return true;
 }
};

// 前進消去のあと、要の列が作る上三角の行列 T (T[t][s] = U_t[pcol[s]]) で、T X = Y を解く。Y の行 t は U_t の列 cols。
// X は要の数 × wx (wx は cols の数を 4 の倍数に切り上げたもの) で、M 未満の値を u64 で持つ。
// X_t = (Y_t - sum_{s>t} T[t][s] X_s) / T[t][t] を t の大きい順に求め、和は 8 本ずつ update_row で足す。
template <class Z> std::vector<u64> tri_solve(const Elim<Z>& e, const std::vector<int>& cols, int& wx) {
 const u32 M= Z::mod();
 const u64 R= (u64(1) << 32) % M;
 const int r= e.prow.size(), nc= cols.size();
 wx= (nc + 3) & ~3;
 std::vector<u64> X((size_t)r * wx), acc(wx), zero(wx);
 for(int t= r - 1; t >= 0; --t) {
  const u64* ut= e.urow(t);
  for(int k= 0; k < nc; ++k) acc[k]= ut[cols[k]];
  for(int k= nc; k < wx; ++k) acc[k]= 0;
  for(int s0= t + 1; s0 < r; s0+= K) {
   const u64* Ub[K];
   u32 f[K];
   for(int q= 0; q < K; ++q) {
    const int s= s0 + q;
    if(s < r) {
     const u32 x= u32(ut[e.pcol[s]]);
     Ub[q]= &X[(size_t)s * wx], f[q]= x ? M - x : 0;
    } else Ub[q]= zero.data(), f[q]= 0;
   }
   update_row(acc.data(), Ub, f, 0, wx, R);
  }
  const Z iv= Z::raw(e.pval[t]).inv();
  u64* xt= &X[(size_t)t * wx];
  for(int k= 0; k < wx; ++k) xt[k]= (Z(acc[k]) * iv).val();
 }
 return X;
}
}  // namespace lazy_elim
