// https://onlinejudge.u-aizu.ac.jp/problems/3168
//
// 提出は次を実装する。
//   struct Solver {
//     Solver(int n, const vector<array<int, 2>> &edges);
//     void run();          // ここだけ測る。
//     i64 answer() const;  // 最小頂点被覆の大きさ
//   };
//
// 入力から「文字が隣り合っていて距離が K 以下の組」を辺にしたグラフを作る
// ところは、どちらの実装でも同じ O(N^3) の仕事になる。比べたいのは被覆の
// 求め方なので、そこはハーネスでやる。
#include "pj.hpp"

#ifndef SUBMISSION_HPP
#define SUBMISSION_HPP "submissions/lib-dulmage-mendelsohn.hpp"
#endif
#include SUBMISSION_HPP

signed main() {
  int n, m, k;
  must_scan(scanf("%d %d %d", &n, &m, &k), 3);
  string c;
  c.reserve(n);
  for (int i = 0; i < n; ++i) c += read_token();

  // 距離は K+1 で頭打ちにしておけば、K を超えたかどうかだけが分かればよい。
  vector<vector<int>> dist(n, vector<int>(n, k + 1));
  for (int i = 0; i < n; ++i) dist[i][i] = 0;
  for (int i = 0; i < m; ++i) {
    int u, v;
    must_scan(scanf("%d %d", &u, &v), 2);
    --u, --v;
    dist[u][v] = dist[v][u] = 1;
  }
  for (int x = 0; x < n; ++x)
    for (int i = 0; i < n; ++i)
      for (int j = 0; j < n; ++j)
        if (dist[i][x] + dist[x][j] < dist[i][j]) dist[i][j] = dist[i][x] + dist[x][j];

  vector<array<int, 2>> edges;
  for (int i = 0; i < n; ++i)
    for (int j = 0; j < i; ++j)
      if (int x = (c[i] - c[j] + 26) % 26; (x == 1 || x == 25) && dist[i][j] <= k)
        edges.push_back({i, j});

  Solver sol(n, edges);

  auto t0 = chrono::steady_clock::now();
  sol.run();
  auto t1 = chrono::steady_clock::now();

  printf("%lld\n", sol.answer());

  // 出力の整形まで含めたピークを読みたいので、計測値は最後に出す。
  report_metrics(
      (long long)chrono::duration_cast<chrono::nanoseconds>(t1 - t0).count());
}
