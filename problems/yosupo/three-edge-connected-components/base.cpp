// https://judge.yosupo.jp/problem/three_edge_connected_components
//
// 提出は次を実装する。
//   struct Solver {
//     Solver(int n, const vector<array<int, 2>> &edges);  // 無向辺の列。自己ループと多重辺を含みうる。edges を覚えるだけにする。
//                                                          // edges は Solver より長く生きる
//     void run();                                          // ここだけ測る。
//     int comp(int v) const;                               // 頂点 v の三辺連結成分の名前。0 以上 n 未満で、同じ成分なら同じ値
//   };
//
// 隣接を組むところと作業領域の確保も run() の中に置く (yosupo-scc と同じ)。comp() は run() の結果から O(1) で返す。成分の名前は
// 詰めなくてよく、0 から K - 1 に振り直して成分ごとに頂点を集めて書く整形は、計測区間の外でハーネスが行う。
// 設計は algo-notes の notes/graph-basics.md。
#include "pj.hpp"

#ifndef SUBMISSION_HPP
#define SUBMISSION_HPP "submissions/tsin.hpp"
#endif
#include SUBMISSION_HPP

signed main() {
  int n, m;
  must_scan(scanf("%d %d", &n, &m), 2);
  vector<array<int, 2>> edges(m);
  for (auto &e : edges) must_scan(scanf("%d %d", &e[0], &e[1]), 2);

  Solver s(n, edges);

  auto t0 = chrono::steady_clock::now();
  s.run();
  auto t1 = chrono::steady_clock::now();

  // 名前を 0 から振り直し、成分ごとに頂点を番号の小さい順に集める。
  vector<int> id(n, -1), comp(n), start(1, 0);
  int k = 0;
  for (int v = 0; v < n; ++v) {
    const int c = s.comp(v);
    if (c < 0 || c >= n) {
      fprintf(stderr, "comp(%d) out of range: %d\n", v, c);
      exit(1);
    }
    if (id[c] < 0) id[c] = k++, start.push_back(0);
    comp[v] = id[c], ++start[id[c] + 1];
  }
  for (int c = 0; c < k; ++c) start[c + 1] += start[c];
  vector<int> pos(start.begin(), start.end() - 1), vs(n);
  for (int v = 0; v < n; ++v) vs[pos[comp[v]]++] = v;

  string out;
  out.reserve(size_t(n + k) * 8 + 16);
  out += to_string(k);
  out += '\n';
  for (int c = 0; c < k; ++c) {
    out += to_string(start[c + 1] - start[c]);
    for (int i = start[c]; i < start[c + 1]; ++i) {
      out += ' ';
      out += to_string(vs[i]);
    }
    out += '\n';
  }
  fwrite(out.data(), 1, out.size(), stdout);

  // 出力の整形まで含めたピークを読みたいので、計測値は最後に出す。
  report_metrics((long long)chrono::duration_cast<chrono::nanoseconds>(t1 - t0).count());
}
