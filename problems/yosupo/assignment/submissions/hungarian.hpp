#pragma once
// Hungarian 法 (行を 1 つずつ足し、そのたびに最短の増加路を Dijkstra で探す形、O(N^3))。行と列にポテンシャル u, v を
// 持ち、足す行から、列ごとの縮約費用の最小 minv と、その最小を与えた直前の列 way を更新しながら、まだ使っていない列の
// うち minv が最小の列へ進む。空いている列に着いたら way を逆に辿って割り当てを入れ替える。行列は行優先の 1 本の
// 配列のまま読む。
#include <limits>
#include <vector>
namespace asg_hungarian {
using i64= long long;
// 行 i の列 p[i] を返し、費用の和を total に入れる。
inline std::vector<int> solve(int n, const std::vector<i64>& a, i64& total) {
 const i64 INF= std::numeric_limits<i64>::max() / 4;
 // 列 0 は番兵。行と列の番号は 1 から。
 std::vector<i64> u(n + 1), v(n + 1), minv(n + 1);
 std::vector<int> p(n + 1), way(n + 1);
 std::vector<char> used(n + 1);
 for(int i= 1; i <= n; ++i) {
  p[0]= i;
  int j0= 0;
  std::fill(minv.begin(), minv.end(), INF);
  std::fill(used.begin(), used.end(), 0);
  do {
   used[j0]= 1;
   const int i0= p[j0];
   const i64* row= a.data() + (size_t)(i0 - 1) * n - 1;
   const i64 ui= u[i0];
   i64 delta= INF;
   int j1= 0;
   for(int j= 1; j <= n; ++j)
    if(!used[j]) {
     const i64 cur= row[j] - ui - v[j];
     if(cur < minv[j]) minv[j]= cur, way[j]= j0;
     if(minv[j] < delta) delta= minv[j], j1= j;
    }
   for(int j= 0; j <= n; ++j)
    if(used[j]) u[p[j]]+= delta, v[j]-= delta;
    else minv[j]-= delta;
   j0= j1;
  } while(p[j0] != 0);
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
 void run() { p= asg_hungarian::solve(n, a, total); }
 i64 cost() const { return total; }
 const vector<int>& assignment() const { return p; }
};
