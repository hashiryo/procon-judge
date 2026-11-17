#pragma once
// cf-1765-a の提出で共有する、入力の読み込みと、文書の鎖から答えを組む部分。
//
// 文書 j に、その文書を見られる開発者の集合 S_j を対応させる。同じ組の文書は、水準の低い順に集合が包含の向きで並ぶ
// (水準 c の文書を見られる開発者は、それより低い水準の文書も見られる) ので、組は包含の順序の鎖になる。逆に、鎖になって
// いれば水準を付けられるので、最小の組の数は、包含の順序を覆う最小の鎖の数になる。順序は、S_j が S_j' を真に含むか、
// 同じ集合で j < j' のときに j を前にする (推移的)。
#include <bitset>
#include <cstdio>
#include <string>
#include <vector>
struct CF1765A {
 int n= 0, m= 0;
 std::vector<std::string> a;
 std::vector<std::bitset<500>> col;
 bool read() {
  if(scanf("%d %d", &n, &m) != 2) return false;
  a.assign(n, std::string());
  static char buf[512];
  for(auto& s: a) {
   if(scanf("%511s", buf) != 1) return false;
   s= buf;
  }
  col.assign(m, {});
  for(int i= 0; i < n; ++i)
   for(int j= 0; j < m; ++j)
    if(a[i][j] == '1') col[j][i]= 1;
  return true;
 }
 // j を j' の前にするか。
 bool before(int j, int k) const {
  if(col[j] == col[k]) return j < k;
  return (col[j] & col[k]) == col[k];
 }
 // 包含の順序の組を全部 (推移的な関係のまま) 列挙する。
 template <class F> void for_each_pair(F&& f) const {
  for(int j= 0; j < m; ++j)
   for(int k= 0; k < m; ++k)
    if(j != k && before(j, k)) f(j, k);
 }
 // 鎖 (前の文書の集合が後の文書の集合を含む) の列から答えを組んで書く。鎖の p 番目 (0 始まり) の水準を p + 2 にし、
 // 開発者の水準は、見られる文書のうち最も後ろの位置 p の p + 2 (見られる文書が無ければ 1) にする。
 void print(const std::vector<std::vector<int>>& chains) const {
  const int k= chains.size();
  std::vector<int> grp(m), lev(m), pos(m);
  for(int g= 0; g < k; ++g)
   for(int p= 0; p < (int)chains[g].size(); ++p) grp[chains[g][p]]= g, lev[chains[g][p]]= p + 2;
  std::string out= std::to_string(k) + '\n';
  for(int j= 0; j < m; ++j) out+= std::to_string(grp[j] + 1), out+= j + 1 < m ? ' ' : '\n';
  for(int j= 0; j < m; ++j) out+= std::to_string(lev[j]), out+= j + 1 < m ? ' ' : '\n';
  for(int i= 0; i < n; ++i) {
   std::vector<int> v(k, 1);
   for(int j= 0; j < m; ++j)
    if(a[i][j] == '1' && lev[j] > v[grp[j]]) v[grp[j]]= lev[j];
   for(int g= 0; g < k; ++g) out+= std::to_string(v[g]), out+= g + 1 < k ? ' ' : '\n';
  }
  fputs(out.c_str(), stdout);
 }
};
