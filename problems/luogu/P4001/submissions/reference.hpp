#pragma once
// 想定解の、平面グラフの双対の最短路 (期待出力を作る参照実装)。格子の正方形 (i, j) を斜めの道で右上と左下の三角形に分け、
// 外側の面を左上と右下の角で、上辺と右辺に接する面 (始点) と、左辺と下辺に接する面 (終点) に分ける。道 1 本は、その両側の
// 面を結ぶ双対の辺になり、左上と右下を分ける道の集まりは始点から終点への双対の道と 1 対 1 に対応する。双対の頂点の隣は
// 添字から求め、Dijkstra を二分ヒープで回す。
// ライブラリも _shared も include しない (期待出力のキャッシュの鍵はこのファイルの中身だけで決まるため)。
#include <functional>
#include <queue>
#include <utility>
#include <vector>
struct Solver {
 int n, m;
 const vector<int>&h, &v, &d;
 long long ans= 0;
 Solver(int n, int m, const vector<int>& h, const vector<int>& v, const vector<int>& d): n(n), m(m), h(h), v(v), d(d) {}
 void run() {
  // 正方形 (i, j) の右上の三角形は 2 (i (m - 1) + j)、左下は 2 (i (m - 1) + j) + 1。外側の面は F (上辺と右辺) と F + 1
  const int w= m - 1, F= 2 * (n - 1) * w, TR= F, BL= F + 1;
  const long long INF= 1LL << 62;
  vector<long long> dist(F + 2, INF);
  priority_queue<pair<long long, int>, vector<pair<long long, int>>, greater<>> pq;
  auto relax= [&](int x, long long c) {
   if(c < dist[x]) dist[x]= c, pq.emplace(c, x);
  };
  dist[TR]= 0;
  for(int j= 0; j < w; ++j) relax(2 * j, h[j]);
  for(int i= 0; i + 1 < n; ++i) relax(2 * (i * w + w - 1), v[i * m + m - 1]);
  while(!pq.empty()) {
   auto [c, x]= pq.top();
   pq.pop();
   if(c != dist[x]) continue;
   if(x == BL) break;
   const int q= x >> 1, i= q / w, j= q % w;
   if(!(x & 1)) {
    // 右上の三角形: 上の横の道、右の縦の道、斜めの道
    relax(i == 0 ? TR : 2 * ((i - 1) * w + j) + 1, c + h[i * w + j]);
    relax(j + 1 == w ? TR : 2 * (i * w + j + 1) + 1, c + v[i * m + j + 1]);
    relax(x + 1, c + d[i * w + j]);
   } else {
    // 左下の三角形: 左の縦の道、下の横の道、斜めの道
    relax(j == 0 ? BL : 2 * (i * w + j - 1), c + v[i * m + j]);
    relax(i + 2 == n ? BL : 2 * ((i + 1) * w + j), c + h[(i + 1) * w + j]);
    relax(x - 1, c + d[i * w + j]);
   }
  }
  ans= dist[BL];
 }
 long long answer() const { return ans; }
};
