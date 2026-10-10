// 自作の問題。一般グラフの最大マッチングの大きさを、性質の違うグラフの族で測る。
//
// 提出は次を実装する。
//   struct Solver {
//     Solver(int n, const vector<array<int, 2>> &edges);  // 無向辺 {u, v}。u != v で、多重辺はありうる
//     void run();                  // 最大マッチングを求める
//     vector<int> answer() const;  // マッチングに使う辺の番号
//   };
//
// 入力は 1 行で、グラフの族の名前、seed、引数を並べる。グラフはハーネスが計測の前に作る。どの族も、頂点の番号を乱択で
// 振り直し (tri の S = 0 を除く)、辺の順と各辺の向きを乱択で並べ替える。構築と run を測る。answer は計測区間の外で呼び、
// ハーネスが辺の番号の範囲と端点が重ならないことを確かめてから、マッチングの大きさを出す (崩れていれば invalid)。
// edges は計測区間のあとまで生きているので、提出は参照を持ってよい。
//
//   random N M        N 頂点に、両端が違う乱択の辺を M 本
//   regular N D       N 頂点の D 正則に近いグラフ (configuration model で、自己ループは捨てる)
//   tri H W P S       H × W の三角格子 (マスの右、下、右下と結ぶ)。各頂点を P % で消して孤立点にする。S = 0 なら頂点の番号を
//                     行優先のまま残す
//   bipartite L R D   左 L 頂点と右 R 頂点の二部グラフ。左の各頂点から乱択の右 D 頂点へ
//   augcycle N W K    左の i と右の i、i + 1 (mod N) を結んだ閉路に、左の i から右の [i - 2W, i - W] の乱択の 1 頂点へ弦を足す
//                     (Library Checker の bipartitematching の augmented_cycle の seed 1 と同じ形)。さらに、左の乱択の i と
//                     [i - W, i) の乱択の左の頂点を結ぶ辺を K 本足して、奇閉路を作る
//   band N D1 D2 K    左の i から右の i - D1 + 1 .. i へ、さらに右の [0, i] の乱択の D2 頂点へ (unique_matching の形)。
//                     左の乱択の 2 頂点を結ぶ辺を K 本足す
//   star N K D        K 個の中心と N - K 個の葉。各葉から乱択の D 個の中心へ、各中心 h から h + 1 (mod K) と乱択の中心へ。
//                     最大マッチングは K 本ほどで、空いた葉からの探索はほとんどが失敗する
//   cliques K S B     S 頂点の完全グラフ K 個を輪に並べ、隣り合う 2 個の間に乱択の辺を B 本張る
#include "pj.hpp"
#include <algorithm>
#include <numeric>
#include <string>

#ifndef SUBMISSION_HPP
#define SUBMISSION_HPP "reference.hpp"
#endif
#include SUBMISSION_HPP

namespace gen {
struct Rng {
  u64 x;
  u64 next() {
    u64 z = (x += 0x9E3779B97F4A7C15ULL);
    z = (z ^ (z >> 30)) * 0xBF58476D1CE4E5B9ULL;
    z = (z ^ (z >> 27)) * 0x94D049BB133111EBULL;
    return z ^ (z >> 31);
  }
  // [0, n)
  int below(u64 n) { return (int)((unsigned __int128)next() * n >> 64); }
  // [lo, hi]
  int range(int lo, int hi) { return lo + below((u64)(hi - lo) + 1); }
};

struct Graph {
  int n = 0;
  bool shuffle = true;
  vector<array<int, 2>> e;
};

Graph random(Rng &rng, int N, int M) {
  Graph g;
  g.n = N;
  g.e.reserve(M);
  while (N >= 2 && (int)g.e.size() < M) {
    const int u = rng.below(N), v = rng.below(N);
    if (u != v) g.e.push_back({u, v});
  }
  return g;
}

Graph regular(Rng &rng, int N, int D) {
  Graph g;
  g.n = N;
  vector<int> stub((size_t)N * D);
  for (size_t i = 0; i < stub.size(); ++i) stub[i] = (int)(i / D);
  for (size_t i = stub.size(); i > 1; --i) swap(stub[i - 1], stub[rng.below(i)]);
  for (size_t i = 0; i + 1 < stub.size(); i += 2)
    if (stub[i] != stub[i + 1]) g.e.push_back({stub[i], stub[i + 1]});
  return g;
}

Graph tri(Rng &rng, int H, int W, int P, int S) {
  Graph g;
  g.n = H * W, g.shuffle = S != 0;
  vector<char> alive(H * W);
  for (auto &a : alive) a = rng.below(100) >= P;
  auto add = [&](int u, int v) {
    if (alive[u] && alive[v]) g.e.push_back({u, v});
  };
  for (int i = 0; i < H; ++i)
    for (int j = 0; j < W; ++j) {
      const int v = i * W + j;
      if (j + 1 < W) add(v, v + 1);
      if (i + 1 < H) add(v, v + W);
      if (i + 1 < H && j + 1 < W) add(v, v + W + 1);
    }
  return g;
}

Graph bipartite(Rng &rng, int L, int R, int D) {
  Graph g;
  g.n = L + R;
  for (int a = 0; a < L; ++a)
    for (int k = 0; k < D; ++k) g.e.push_back({a, L + rng.below(R)});
  return g;
}

Graph augcycle(Rng &rng, int N, int W, int K) {
  Graph g;
  g.n = 2 * N;
  for (int i = 0; i < N; ++i) {
    g.e.push_back({i, N + i});
    g.e.push_back({i, N + (i + 1) % N});
    g.e.push_back({i, N + rng.range(max(0, i - 2 * W), max(0, i - W))});
  }
  for (int k = 0; k < K; ++k) {
    const int i = rng.range(1, N - 1);
    g.e.push_back({i, rng.range(max(0, i - W), i - 1)});
  }
  sort(g.e.begin(), g.e.end());
  g.e.erase(unique(g.e.begin(), g.e.end()), g.e.end());
  return g;
}

Graph band(Rng &rng, int N, int D1, int D2, int K) {
  Graph g;
  g.n = 2 * N;
  for (int i = 0; i < N; ++i) {
    for (int j = max(0, i - D1 + 1); j <= i; ++j) g.e.push_back({i, N + j});
    for (int k = 0; k < D2; ++k) g.e.push_back({i, N + rng.below(i + 1)});
  }
  for (int k = 0; k < K; ++k) {
    const int u = rng.below(N), v = rng.below(N);
    if (u != v) g.e.push_back({u, v});
  }
  return g;
}

Graph star(Rng &rng, int N, int K, int D) {
  Graph g;
  g.n = N;
  for (int v = K; v < N; ++v)
    for (int k = 0; k < D; ++k) g.e.push_back({v, rng.below(K)});
  for (int h = 0; h < K; ++h) {
    if (K > 1) g.e.push_back({h, (h + 1) % K});
    const int x = rng.below(K);
    if (x != h) g.e.push_back({h, x});
  }
  return g;
}

Graph cliques(Rng &rng, int K, int S, int B) {
  Graph g;
  g.n = K * S;
  for (int c = 0; c < K; ++c) {
    for (int a = 0; a < S; ++a)
      for (int b = a + 1; b < S; ++b) g.e.push_back({c * S + a, c * S + b});
    const int d = (c + 1) % K;
    if (d != c)
      for (int k = 0; k < B; ++k) g.e.push_back({c * S + rng.below(S), d * S + rng.below(S)});
  }
  return g;
}
}  // namespace gen

signed main() {
  const string fam = read_token();
  long long seed;
  must_scan(scanf("%lld", &seed), 1);
  gen::Rng rng{(u64)seed};
  auto args = [&](int k) {
    vector<int> a(k);
    for (auto &x : a) must_scan(scanf("%d", &x), 1);
    return a;
  };
  gen::Graph g;
  if (fam == "random") { auto a = args(2); g = gen::random(rng, a[0], a[1]); }
  else if (fam == "regular") { auto a = args(2); g = gen::regular(rng, a[0], a[1]); }
  else if (fam == "tri") { auto a = args(4); g = gen::tri(rng, a[0], a[1], a[2], a[3]); }
  else if (fam == "bipartite") { auto a = args(3); g = gen::bipartite(rng, a[0], a[1], a[2]); }
  else if (fam == "augcycle") { auto a = args(3); g = gen::augcycle(rng, a[0], a[1], a[2]); }
  else if (fam == "band") { auto a = args(4); g = gen::band(rng, a[0], a[1], a[2], a[3]); }
  else if (fam == "star") { auto a = args(3); g = gen::star(rng, a[0], a[1], a[2]); }
  else if (fam == "cliques") { auto a = args(3); g = gen::cliques(rng, a[0], a[1], a[2]); }
  else {
    fprintf(stderr, "unknown family %s\n", fam.c_str());
    return 1;
  }
  if (g.shuffle) {
    vector<int> p(g.n);
    iota(p.begin(), p.end(), 0);
    for (int i = g.n; i > 1; --i) swap(p[i - 1], p[rng.below(i)]);
    for (auto &e : g.e) e = {p[e[0]], p[e[1]]};
  }
  for (size_t i = g.e.size(); i > 1; --i) swap(g.e[i - 1], g.e[rng.below(i)]);
  for (auto &e : g.e)
    if (rng.next() >> 63) swap(e[0], e[1]);

  auto t0 = chrono::steady_clock::now();
  Solver sol(g.n, g.e);
  sol.run();
  auto t1 = chrono::steady_clock::now();

  const vector<int> ans = sol.answer();
  const int m = g.e.size();
  vector<char> used(g.n);
  string bad;
  for (int i : ans) {
    if (i < 0 || i >= m) {
      bad = "edge id out of range";
      break;
    }
    auto [u, v] = g.e[i];
    if (used[u] || used[v]) {
      bad = "vertex used twice";
      break;
    }
    used[u] = used[v] = 1;
  }
  if (bad.empty()) printf("%d\n", (int)ans.size());
  else printf("invalid: %s\n", bad.c_str());

  report_metrics(
      (long long)chrono::duration_cast<chrono::nanoseconds>(t1 - t0).count());
}
