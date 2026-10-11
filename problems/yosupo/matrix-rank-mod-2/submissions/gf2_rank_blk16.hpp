#pragma once
// 要を 16 本ずつまとめて掃き出す GF(2) の核 (_shared/linalg/gf2_elim.hpp) で階数を求める。
// 行は 4 語 (256 bit) の倍数に揃えるので、列が 64 以下で縦に長い行列は転置する (1 列の行列で行が 2^24 本になると 512 MB 要る)。
// それ以外は転置しない (4097 × 4095 のような行列で転置すると、入力の 1 行ごとに 4095 本の行へ 1 bit ずつ書くことになって遅い)。
#include "../common.hpp"
#include "_shared/linalg/gf2_elim.hpp"
inline int run(int n, int m, const vector<string>& a) {
 const bool tr= n > m && m <= 64;
 const int rows= tr ? m : n, cols= tr ? n : m;
 if(rows == 0) return 0;
 gf2_elim::Elim e(rows, ((cols + 63) / 64 + 3) & ~3);
 if(!tr)
  for(int i= 0; i < n; ++i) gf2_elim::pack_row(a[i].data(), m, e.row(i));
 else {  // 64 行分の bit を列ごとにレジスタにためてから書く
  u64 acc[64]= {};
  for(int i= 0; i < n; ++i) {
   const char* s= a[i].data();
   for(int j= 0; j < m; ++j) acc[j]|= u64(s[j] & 1) << (i & 63);
   if((i & 63) == 63 || i + 1 == n)
    for(int j= 0; j < m; ++j) e.row(j)[i >> 6]= acc[j], acc[j]= 0;
  }
 }
 e.run(cols, false);
 return e.prow.size();
}
