// https://judge.yosupo.jp/problem/scc
//
// 提出は次を実装する。
//   struct Solver {
//     Solver(int n, const vector<array<int, 2>> &edges);  // 辺 (a, b) の列を覚えるだけにする。edges は Solver より長く生きる
//     void run();                                          // ここだけ測る。
//     int count() const;                                   // 成分の数 K
//     int comp(int v) const;                               // 頂点 v の成分の番号。0 以上 K 未満
//   };
//
// 成分の番号は、成分を縮めた DAG のトポロジカル順に振る。どの辺 (a, b) についても comp(a) <= comp(b) になる。
// 隣接リストを組むところと作業領域の確保も run() の中に置く。持ち方が実装で違うので、そこも比較の対象になる
// (aoj-GRL_6_A と同じ)。count() と comp() は run() の結果から O(1) で返す。見つけた順を逆にするような番号の
// 付け替えはここでしてよい。成分ごとに頂点を集めて書く整形は、計測区間の外でハーネスが行う。
// 設計は algo-notes の notes/graph-basics.md。
#include "pj.hpp"

#ifndef SUBMISSION_HPP
#define SUBMISSION_HPP "submissions/lib.hpp"
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

  // 成分ごとに、頂点を番号の小さい順に集める。
  const int k = s.count();
  if (k < 1 || k > n) {
    fprintf(stderr, "count() out of range: %d\n", k);
    exit(1);
  }
  vector<int> comp(n), start(k + 1);
  for (int v = 0; v < n; ++v) {
    const int c = s.comp(v);
    if (c < 0 || c >= k) {
      fprintf(stderr, "comp(%d) out of range: %d\n", v, c);
      exit(1);
    }
    comp[v] = c, ++start[c + 1];
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
