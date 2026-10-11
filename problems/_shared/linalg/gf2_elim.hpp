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
#include <array>
#include <cstring>
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

// x[w..W) ^= y[w..W)、x[w..W) ^= y ^ z、x[w..W) = y ^ z、x[w..W) = y。W は 4 の倍数で、w は 4 の倍数とは限らない (端数は 1 語ずつ)。
// GCC が restrict の付いたループでもベクトル化しないことがあったので、AVX2 で書く。
inline void xor_in(u64* x, const u64* y, int w, int W) {
 for(; w & 3; ++w) x[w]^= y[w];
 for(; w < W; w+= 4) _mm256_storeu_si256((__m256i*)(x + w), _mm256_xor_si256(_mm256_loadu_si256((const __m256i*)(x + w)), _mm256_loadu_si256((const __m256i*)(y + w))));
}
inline void xor2_in(u64* x, const u64* y, const u64* z, int w, int W) {
 for(; w & 3; ++w) x[w]^= y[w] ^ z[w];
 for(; w < W; w+= 4)
  _mm256_storeu_si256((__m256i*)(x + w), _mm256_xor_si256(_mm256_loadu_si256((const __m256i*)(x + w)), _mm256_xor_si256(_mm256_loadu_si256((const __m256i*)(y + w)), _mm256_loadu_si256((const __m256i*)(z + w)))));
}
inline void xor_to(u64* x, const u64* y, const u64* z, int w, int W) {
 for(; w & 3; ++w) x[w]= y[w] ^ z[w];
 for(; w < W; w+= 4) _mm256_storeu_si256((__m256i*)(x + w), _mm256_xor_si256(_mm256_loadu_si256((const __m256i*)(y + w)), _mm256_loadu_si256((const __m256i*)(z + w))));
}
inline void copy_to(u64* x, const u64* y, int w, int W) {
 for(; w & 3; ++w) x[w]= y[w];
 for(; w < W; w+= 4) _mm256_storeu_si256((__m256i*)(x + w), _mm256_loadu_si256((const __m256i*)(y + w)));
}

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

// 語 src の bit [b0, b0 + m) を、'0' と '1' の文字 m 個にして dst に書く。
inline void unpack_bits(const u64* src, int b0, int m, char* dst) {
 static const auto tbl= [] {  // 8 bit から 8 文字
  std::array<u64, 256> t{};
  for(int x= 0; x < 256; ++x)
   for(int i= 0; i < 8; ++i) t[x]|= u64('0' + (x >> i & 1)) << (8 * i);
  return t;
 }();
 int j= 0;
 if(!(b0 & 7))
  for(; j + 8 <= m; j+= 8) {
   const int p= b0 + j;
   const u64 v= tbl[src[p >> 6] >> (p & 63) & 255];
   std::memcpy(dst + j, &v, 8);
  }
 for(; j < m; ++j) {
  const int p= b0 + j;
  dst[j]= char('0' + (src[p >> 6] >> (p & 63) & 1));
 }
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
    copy_to(up, ap, wi, W);
    for(u32 m= f[p]; m; m&= m - 1) xor_in(up, Ub[__builtin_ctz(m)], wi, W);
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
 static bool bit(const u64* x, int j) { return x[j >> 6] >> (j & 63) & 1; }
 // 前進消去のあと、要の行を RREF に直す (U の行 t が、要の列 pcol[t] で 1、ほかの要の列で 0 になる)。要の行を下から 16 本ずつ
 // 組にし、組の中を下から消してから、組の要の行の 2 枚の表で、それより上の要の行の組の要の列を消す。
 void back() {
  const int r= prow.size();
  if(!r) return;
  const int k1= std::min(r, 8), k2= std::min(std::max(r - 8, 0), 8);
  std::vector<u64> T1((size_t)W << k1), T2(k2 ? (size_t)W << k2 : 0);
  for(int t1= r; t1 > 0; t1-= K) {
   const int t0= std::max(0, t1 - K), bt= t1 - t0, w0= pcol[t0] >> 6;
   for(int t= t1 - 2; t >= t0; --t) {
    u64* x= &U[(size_t)t * W];
    for(int s= t + 1; s < t1; ++s)
     if(bit(x, pcol[s])) xor_in(x, urow(s), pcol[s] >> 6, W);
   }
   const u64* Ub[K];
   for(int s= 0; s < bt; ++s) Ub[s]= urow(t0 + s);
   auto build= [&](std::vector<u64>& T, int s0, int cnt) {
    std::fill(&T[w0], &T[W], 0);
    for(int m= 1; m < (1 << cnt); ++m) xor_to(&T[(size_t)m * W], &T[(size_t)(m & (m - 1)) * W], Ub[s0 + __builtin_ctz(m)], w0, W);
   };
   build(T1, 0, std::min(bt, 8));
   if(bt > 8) build(T2, 8, bt - 8);
   const bool consecutive= pcol[t1 - 1] - pcol[t0] == bt - 1;
   for(int t= 0; t < t0; ++t) {
    u64* x= &U[(size_t)t * W];
    u32 m= 0;
    if(consecutive) {  // 組の要の列が続いているなら、2 語から 16 bit をまとめて取る
     const int j= pcol[t0], wj= j >> 6, b= j & 63;
     u64 v= x[wj] >> b;
     if(b && wj + 1 < W) v|= x[wj + 1] << (64 - b);
     m= u32(v) & ((1u << bt) - 1);
    } else
     for(int s= 0; s < bt; ++s) m|= u32(bit(x, pcol[t0 + s])) << s;
    if(!m) continue;
    if(m >> 8) xor2_in(x, &T1[(size_t)(m & 255) * W], &T2[(size_t)(m >> 8) * W], w0, W);
    else xor_in(x, &T1[(size_t)(m & 255) * W], w0, W);
   }
  }
 }
 // 貯めた bt 本の要の更新を、残りの行の語 [wb, W) に足す。
 void flush(const u64* const* Ub, int bt, int wb, std::vector<u32>& f, std::vector<u64>& T1, std::vector<u64>& T2) {
  auto build= [&](std::vector<u64>& T, int s0, int cnt) {
   std::fill(&T[wb], &T[W], 0);
   for(int m= 1; m < (1 << cnt); ++m) xor_to(&T[(size_t)m * W], &T[(size_t)(m & (m - 1)) * W], Ub[s0 + __builtin_ctz(m)], wb, W);
  };
  build(T1, 0, std::min(bt, 8));
  if(bt > 8) build(T2, 8, bt - 8);
  for(int r : rem) {
   const u32 m= f[r];
   if(!m) continue;
   f[r]= 0;
   if(m >> 8) xor2_in(row(r), &T1[(size_t)(m & 255) * W], &T2[(size_t)(m >> 8) * W], wb, W);
   else xor_in(row(r), &T1[(size_t)(m & 255) * W], wb, W);
  }
 }
};
}  // namespace gf2_elim
