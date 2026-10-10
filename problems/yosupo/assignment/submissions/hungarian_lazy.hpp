#pragma once
// hungarian の 1 歩ごとの 2 回目の走査 (使った列のポテンシャルを動かし、使っていない列の minv を下げる) を無くしたもの。
// 1 行を足す間に動かした量の累計 T を持ち、minv には T を足した値を入れておく (minv から delta を引く代わりに T が増える)。
// 列 j を使った時点の T を tj[j] に覚え、行を足し終えたところで、使った列ごとに T - tj[j] だけポテンシャルを動かす。
// 行 i0 は、その行の列を使った歩で初めて木に入るので、その歩で縮約費用を計算するときの u[i0] はまだ動いていない。
// 使った列は veff に大きな負の値を入れて候補から外し、minv にはどの候補よりも大きい値を入れて最小に選ばれないように
// する。これで内側の走査は分岐の無い 1 回になる。
#include <vector>
namespace asg_hungarian_lazy {
using i64= long long;
inline std::vector<int> solve(int n, const std::vector<i64>& a, i64& total) {
 // 費用は 10^9 以下、ポテンシャルと T は 10^13 程度に収まるので、2^60 で候補から外し、その半分で最小から外す。
 constexpr i64 OFF= i64(1) << 60, USED= OFF / 2, INF= OFF / 2;
 // 列 0 は番兵。行と列の番号は 1 から。
 std::vector<i64> u(n + 1), v(n + 1), veff(n + 1), minv(n + 1), tj(n + 1);
 std::vector<int> p(n + 1), way(n + 1), usedcols;
 usedcols.reserve(n + 1);
 for(int i= 1; i <= n; ++i) {
  p[0]= i;
  int j0= 0;
  i64 T= 0;
  for(int j= 0; j <= n; ++j) veff[j]= v[j], minv[j]= INF;
  usedcols.clear();
  do {
   usedcols.push_back(j0);
   tj[j0]= T, veff[j0]= -OFF, minv[j0]= USED;
   const int i0= p[j0];
   const i64* row= a.data() + (size_t)(i0 - 1) * n - 1;
   const i64 c= T - u[i0];
   i64 best= INF;
   int j1= 0;
   for(int j= 1; j <= n; ++j) {
    const i64 cur= row[j] - veff[j] + c;
    if(cur < minv[j]) minv[j]= cur, way[j]= j0;
    if(minv[j] < best) best= minv[j], j1= j;
   }
   T= best;
   j0= j1;
  } while(p[j0] != 0);
  for(int j: usedcols) {
   const i64 d= T - tj[j];
   u[p[j]]+= d, v[j]-= d;
  }
  do {
   const int j1= way[j0];
   p[j0]= p[j1], j0= j1;
  } while(j0);
 }
 std::vector<int> ans(n);
 total= 0;
 for(int j= 1; j <= n; ++j) ans[p[j] - 1]= j - 1;
 for(int i= 0; i < n; ++i) total+= a[(size_t)i * n + ans[i]];
 return ans;
}
}
struct Solver {
 int n;
 const vector<i64>& a;
 i64 total= 0;
 vector<int> p;
 Solver(int n, const vector<i64>& a): n(n), a(a) {}
 void run() { p= asg_hungarian_lazy::solve(n, a, total); }
 i64 cost() const { return total; }
 const vector<int>& assignment() const { return p; }
};
