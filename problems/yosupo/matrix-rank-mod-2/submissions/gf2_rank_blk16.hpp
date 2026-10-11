#pragma once
// 要を 16 本ずつまとめて掃き出す GF(2) の核 (_shared/linalg/gf2_elim.hpp) で階数を求める。
// 行の数が min(N, M) になる向きに置く (縦に長い行列は転置して 1 本の長い行にする)。
#include "../common.hpp"
#include "_shared/linalg/gf2_elim.hpp"
inline int run(int n, int m, const vector<string>& a) {
 const bool tr= n > m;
 const int rows= tr ? m : n, cols= tr ? n : m;
 if(rows == 0) return 0;
 gf2_elim::Elim e(rows, ((cols + 63) / 64 + 3) & ~3);
 if(!tr)
  for(int i= 0; i < n; ++i) gf2_elim::pack_row(a[i].data(), m, e.row(i));
 else
  for(int i= 0; i < n; ++i) {
   const char* s= a[i].data();
   for(int j= 0; j < m; ++j)
    if(s[j] & 1) e.row(j)[i >> 6]|= u64(1) << (i & 63);
  }
 e.run(cols, false);
 return e.prow.size();
}
