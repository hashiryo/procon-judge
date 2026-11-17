// NeoLibrary の euler_split (neo/graph/euler_split.hpp) で解く。配列を左、値を右の頂点にして、要素ごとに辺を張る。どの値も全体で
// 偶数回出てくれば、二部グラフで次数が全部偶数なので、Euler 分割で配列ごとにも値ごとにも辺がちょうど半分ずつに分かれる。色 0 を
// L、1 を R にする。奇数回の値があれば NO。
#include <algorithm>
#include <array>
#include <cstdio>
#include <string>
#include <vector>
#include "neo/graph/euler_split.hpp"
int main() {
 int m;
 if(scanf("%d", &m) != 1) return 1;
 std::vector<int> len(m), val, who;
 for(int i= 0; i < m; ++i) {
  if(scanf("%d", &len[i]) != 1) return 1;
  for(int j= 0; j < len[i]; ++j) {
   int a;
   if(scanf("%d", &a) != 1) return 1;
   val.push_back(a), who.push_back(i);
  }
 }
 std::vector<int> vs= val;
 std::sort(vs.begin(), vs.end());
 vs.erase(std::unique(vs.begin(), vs.end()), vs.end());
 const int k= vs.size(), total= val.size();
 std::vector<int> cnt(k, 0);
 std::vector<std::array<int, 2>> es(total);
 for(int t= 0; t < total; ++t) {
  const int id= std::lower_bound(vs.begin(), vs.end(), val[t]) - vs.begin();
  ++cnt[id], es[t]= {who[t], m + id};
 }
 for(int c: cnt)
  if(c & 1) return puts("NO"), 0;
 const std::vector<int> col= euler_split(m + k, es);
 std::string out= "YES\n";
 out.reserve(total + m + 8);
 for(int t= 0; t < total; ++t) {
  out+= col[t] ? 'R' : 'L';
  if(t + 1 == total || who[t + 1] != who[t]) out+= '\n';
 }
 fwrite(out.data(), 1, out.size(), stdout);
}
