#pragma once
// 愚直解 (提出ではない)。左上を含み右下を含まない頂点の集合を全部試し、境界を渡る道の重みの和の最小を取る。小さい入力
// (N, M ≤ 4) でだけ使う (pj testdata crosscheck --brute brute.hpp)。
#include <array>
#include <vector>
struct Solver {
 int n, m;
 const vector<int>&h, &v, &d;
 long long ans= 0;
 Solver(int n, int m, const vector<int>& h, const vector<int>& v, const vector<int>& d): n(n), m(m), h(h), v(v), d(d) {}
 void run() {
  vector<array<int, 3>> e;
  for(int i= 0; i < n; ++i)
   for(int j= 0; j < m; ++j) {
    const int x= i * m + j;
    if(j + 1 < m) e.push_back({x, x + 1, h[i * (m - 1) + j]});
    if(i + 1 < n) e.push_back({x, x + m, v[i * m + j]});
    if(i + 1 < n && j + 1 < m) e.push_back({x, x + m + 1, d[i * (m - 1) + j]});
   }
  const int k= n * m - 2;
  ans= -1;
  for(int mask= 0; mask < (1 << k); ++mask) {
   // 頂点 0 (左上) は必ず左上の側、頂点 n m - 1 (右下) は必ず反対側、ほかは mask の bit で決める
   auto side= [&](int x) { return x == 0 ? 1 : x == n * m - 1 ? 0 : (mask >> (x - 1)) & 1; };
   long long cut= 0;
   for(auto [a, b, c]: e)
    if(side(a) != side(b)) cut+= c;
   if(ans < 0 || cut < ans) ans= cut;
  }
 }
 long long answer() const { return ans; }
};
