#pragma once
// 要を 16 本ずつまとめて掃き出す GF(2) の核 (_shared/linalg/gf2_elim.hpp) で Ax = b を解く。
// [A | b] を前進消去し、残りの行の b の bit が立っていれば解なし。要の行を RREF に直すと、特解は要の列に b の bit を置いたもの、
// 自由な列 g の基底は g に 1、要の列 pcol[t] に行 t の列 g の bit を置いたものになる。
#include "../common.hpp"
#include "_shared/linalg/gf2_elim.hpp"
inline vector<string> run(int n, int m, const vector<string>& a, const vector<int>& b) {
 using gf2_elim::Elim;
 Elim e(n, ((m + 1 + 63) / 64 + 3) & ~3);
 for(int i= 0; i < n; ++i) {
  gf2_elim::pack_row(a[i].data(), m, e.row(i));
  if(b[i]) e.row(i)[m >> 6]|= u64(1) << (m & 63);
 }
 e.run(m, false);
 for(int r : e.rem)
  if(Elim::bit(e.row(r), m)) return {};
 e.back();
 const int r= e.prow.size();
 vector<char> isp(m);
 for(int c : e.pcol) isp[c]= 1;
 vector<string> res;
 res.reserve(m - r + 1);
 res.emplace_back(m, '0');
 for(int t= 0; t < r; ++t)
  if(Elim::bit(e.urow(t), m)) res[0][e.pcol[t]]= '1';
 for(int g= 0; g < m; ++g) {
  if(isp[g]) continue;
  string& v= res.emplace_back(m, '0');
  v[g]= '1';
  for(int t= 0; t < r; ++t)
   if(Elim::bit(e.urow(t), g)) v[e.pcol[t]]= '1';
 }
 return res;
}
