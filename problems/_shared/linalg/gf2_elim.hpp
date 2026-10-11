#pragma once
// GF(2) の行列の前進消去を、要を 16 本ずつまとめて行う核 (M4RI の形)。yosupo の mod 2 の問題の提出が読む。
// 記録は algo-notes の notes/linear-algebra.md。
//
// 行は 64 bit の語の列 (語の数は 4 の倍数) で持つ。要を 1 本選ぶたびに残りの行を全部 XOR する代わりに、組の中では
// 行ごとに「どの要の行を足すか」の印 (16 bit) だけを貯める。要を探す列の bit は、行の bit と、印と要の行のその列の bit の
// 論理積の偶奇から求める。16 本たまったら、要の行 8 本ずつの 256 通りの XOR の表を 2 枚作り (Gray 符号の順に 1 本ずつ足す)、
// 残りの各行に表の行を 1 本ずつ XOR する。行列の読み書きは、要 16 本につき 1 回になる。
// 要を探す列の語は、残りの行の分を 64 列ごとに 1 本の配列に集めてから読む (行の間隔ごとに飛ぶ読み出しを 64 分の 1 にする)。
#include <algorithm>
#include <string>
#include <vector>
#ifdef USE_SIMDE
#include <simde/x86/avx2.h>
#else
#include <immintrin.h>
#endif
namespace gf2_elim {
using u32= unsigned;
using u64= unsigned long long;
constexpr int K= 16;

// '0' と '1' の文字列 s (長さ m) を、語 dst に bit で詰める (j 文字目が語 j / 64 の bit j % 64)。
inline void pack_row(const char* s, int m, u64* dst) {
 int j= 0;
 for(; j + 32 <= m; j+= 32) {
  const __m256i c= _mm256_loadu_si256((const __m256i*)(s + j));
  const u32 bits= (u32)_mm256_movemask_epi8(_mm256_slli_epi16(c, 7));
  dst[j >> 6]|= u64(bits) << (j & 63);
 }
 for(; j < m; ++j)
  if(s[j] & 1) dst[j >> 6]|= u64(1) << (j & 63);
}

// n 行、語の数 W (4 の倍数) の行列の前進消去。要は列の順に探し、その列で bit が立っている最初の行を選ぶ。
struct Elim {
 int n, W;
 std::vector<u64> a;  // n × W
 std::vector<int> rem;  // まだ要にしていない行
 std::vector<int> prow, pcol;  // 要にした行と列 (要にした順)
 std::vector<u64> U;  // 要の行 (要にした順に W 語ずつ)。それより前の要で消したあとの値で、要の列の語より左は使わない
 Elim(int n_, int W_): n(n_), W(W_), a((size_t)n_ * W_), rem(n_) {
  for(int i= 0; i < n; ++i) rem[i]= i;
 }
 u64* row(int r) { return &a[(size_t)r * W]; }
 const u64* urow(int t) const { return &U[(size_t)t * W]; }
 // 列 [0, lim) で要を探しながら前進消去する。要のない列があれば、stop なら false を返して止め、そうでなければ飛ばす。
 bool run(int lim, bool stop) {
  U.assign((size_t)std::min(n, lim) * W, 0);
  std::vector<u32> f(n);
  // 表の行は、組に入る要の数 (行の数を超えない) の分だけ取る。行の数が少なく行が長い行列で、表が大きくなりすぎないように。
  const int k1= std::min(n, 8), k2= std::min(std::max(n - 8, 0), 8);
  std::vector<u64> cw(n), T1((size_t)W << k1), T2(k2 ? (size_t)W << k2 : 0);
  const u64* Ub[K];
  int col= 0, cwi= -1;
  while(col < lim && !rem.empty()) {
   int bt= 0;
   for(; bt < K && col < lim && !rem.empty(); ++col) {
    const int wi= col >> 6, b= col & 63, nr= rem.size();
    if(wi != cwi) {
     for(int i= 0; i < nr; ++i) cw[i]= a[(size_t)rem[i] * W + wi];
     cwi= wi;
    }
    u32 P= 0;
    for(int s= 0; s < bt; ++s) P|= u32(Ub[s][wi] >> b & 1) << s;
    int pi= -1;
    for(int i= 0; i < nr; ++i)
     if((cw[i] >> b ^ __builtin_parity(f[rem[i]] & P)) & 1) {
      pi= i;
      break;
     }
    if(pi < 0) {
     if(stop) return false;
     continue;
    }
    const int p= rem[pi];
    u64* up= &U[prow.size() * W];
    const u64* ap= row(p);
    for(int w= wi; w < W; ++w) up[w]= ap[w];
    for(u32 m= f[p]; m; m&= m - 1) {
     const u64* __restrict us= Ub[__builtin_ctz(m)];
     for(int w= wi; w < W; ++w) up[w]^= us[w];
    }
    for(int i= pi + 1; i < nr; ++i) {  // pi より前の行は、この列の bit が 0 だった
     const int r= rem[i];
     f[r]|= u32((cw[i] >> b ^ __builtin_parity(f[r] & P)) & 1) << bt;
    }
    prow.push_back(p), pcol.push_back(col);
    Ub[bt++]= up;
    rem[pi]= rem.back(), rem.pop_back(), cw[pi]= cw[nr - 1];
   }
   if(!bt) break;
   flush(Ub, bt, col >> 6, f, T1, T2);
   cwi= -1;
  }
  return true;
 }
 // 貯めた bt 本の要の更新を、残りの行の語 [wb, W) に足す。
 void flush(const u64* const* Ub, int bt, int wb, std::vector<u32>& f, std::vector<u64>& T1, std::vector<u64>& T2) {
  auto build= [&](std::vector<u64>& T, int s0, int cnt) {
   std::fill(&T[wb], &T[W], 0);
   for(int m= 1; m < (1 << cnt); ++m) {
    u64* __restrict t= &T[(size_t)m * W];
    const u64* __restrict prev= &T[(size_t)(m & (m - 1)) * W];
    const u64* __restrict us= Ub[s0 + __builtin_ctz(m)];
    for(int w= wb; w < W; ++w) t[w]= prev[w] ^ us[w];
   }
  };
  build(T1, 0, std::min(bt, 8));
  if(bt > 8) build(T2, 8, bt - 8);
  for(int r : rem) {
   const u32 m= f[r];
   if(!m) continue;
   f[r]= 0;
   u64* __restrict x= row(r);
   const u64* __restrict t1= &T1[(size_t)(m & 255) * W];
   if(m >> 8) {
    const u64* __restrict t2= &T2[(size_t)(m >> 8) * W];
    for(int w= wb; w < W; ++w) x[w]^= t1[w] ^ t2[w];
   } else
    for(int w= wb; w < W; ++w) x[w]^= t1[w];
  }
 }
};
}  // namespace gf2_elim
