#pragma once
// Library Checker の提出 216351 (https://judge.yosupo.jp/submission/216351) の写し。C++ で 2 番目に速い 0.608 s
// (2026-11-02 に見た順位。首位の 388507 は Library Checker の参照解と同じ多項式の再帰で 0.605 s)。
// 頂点 2p と 2p+1 を組 p とし、組の部分集合 S と、いま開いている閉路の端 j ごとの重み dp[S][j] を、S の小さい順に配る
// (完全マッチングと組の辺を重ねた交互の閉路を、最大の組の小さい順に 1 本ずつ作る)。剰余は積ごとに % で取る。
// 入出力は base.cpp に合わせ、手順は元の提出のまま。
#include "../common.hpp"
namespace yosupo_216351 {
using idt= std::size_t;
constexpr u32 mod= MOD;
constexpr u32 mul(u32 x, u32 y) { return u64(x) * y % mod; }
constexpr u32 shrk(u32 x, u32 M= mod) { return std::min(x, x - M); }
constexpr void rfma(u32& a, u32 b, u32 c) { a= (a + u64(b) * c) % mod; }
constexpr void radd(u32& a, u32 b) { a= shrk(a + b); }
}  // namespace yosupo_216351
inline u32 run(int N_, const vector<vector<u32>>& a) {
 using namespace yosupo_216351;
 const idt N= N_;
 const idt N2= N / 2;
#define BS(x) (idt(1) << (x))
 vector<vector<u32>> dp(BS(N2), vector<u32>(N));
 for(idt i= 0; i < N2; ++i) dp[BS(i)][2 * i]= 1;
 u32 ans= 0;
 for(idt w= 0; w < N2; ++w) {
  for(idt i= BS(w); i < BS(w + 1); ++i) {
   for(idt j= 0; j < 2 * w + 2; ++j) {
    if(!(i & BS(j / 2))) continue;
    for(idt k= 0; k < 2 * w; ++k) {
     if(i & BS(k / 2)) continue;
     rfma(dp[i | BS(k / 2)][k ^ 1], dp[i][j], a[j][k]);
    }
    u32 prd= mul(dp[i][j], a[j][2 * w + 1]);
    for(idt k= w + 1; k < N2; ++k) radd(dp[i | BS(k)][2 * k], prd);
    if(i + 1 == BS(N2)) radd(ans, prd);
   }
  }
 }
#undef BS
 return ans;
}
