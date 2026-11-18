// 自作の問題。グラフ的マトロイドと分割マトロイドの重み付きの交差を、性質の違う族で測る。辺に色と重みがあり、色 c の辺は
// cap[c] 本まで使える。閉路を作らない辺の集合のうち、要素数が k のものの重みの和の最大を、k = 0, 1, ..., r (r は要素数の
// 最大) の全部で求める (大きさ k ごとの最適値。gym 103373I や Kattis rainbowgraph の形)。
//
// 提出は次を実装する。
//   struct Solver {
//     Solver(int n, const vector<array<int, 2>> &edges, const vector<int> &color, const vector<int> &cap,
//            const vector<i64> &w);
//     void run();
//     vector<i64> answer() const;  // answer()[k] は要素数 k の重みの和の最大 (k = 0..r、長さ r + 1)
//     vector<int> chosen() const;  // 要素数 r で重みの和が answer()[r] の集合 (辺の番号)
//   };
// n は頂点数。辺 {u, v} は u != v で、多重辺はありうる。color[i] は辺 i の色で 0 以上 cap.size() 未満、cap[c] >= 0。
// w[i] は 1 以上の整数。
//
// 入力は 1 行で、族の名前、seed、引数を並べる。最後の引数 Wt は重みの上限で、重みは [1, Wt] から一様に選ぶ (Wt が小さいと
// 同じ重みが多くなる)。グラフはハーネスが計測の前に作り、頂点の番号、辺の順、各辺の向きを乱択で並べ替える。構築と run を
// 測る。answer と chosen は計測区間の外で呼ぶ。ハーネスは chosen が森で色ごとの上限を守り、要素数が r で重みの和が
// answer()[r] と等しいことを確かめてから、r と answer() を出す (崩れていれば invalid)。
//
//   random N M C K Wt   N 頂点に乱択の辺を M 本。色は C 色から一様に選び、どの色も上限 K
//   grid H W C K Wt     H × W の格子の辺。色は C 色から一様に選び、どの色も上限 K
//   vertex N M K Wt     N 頂点に乱択の辺を M 本。辺を乱択の端点に割り当て、その頂点を色とする (頂点ごとに K 本まで)
//   few N M C Wt        N 頂点に乱択の辺を M 本。色は C 色から一様に選び、上限は N - 1 を C 個の正の数に乱択で分けたもの
//   complete N Wt       N 頂点の完全グラフ。色は N - 1 色から一様に選び、どの色も上限 1
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

struct Instance {
  int n = 0;
  vector<array<int, 2>> e;
  vector<int> color, cap;
  vector<i64> w;
};

void random_edges(Rng &rng, Instance &g, int N, int M) {
  g.n = N;
  g.e.reserve(M);
  while (N >= 2 && (int)g.e.size() < M) {
    const int u = rng.below(N), v = rng.below(N);
    if (u != v) g.e.push_back({u, v});
  }
}

void uniform_colors(Rng &rng, Instance &g, int C, int K) {
  g.color.resize(g.e.size());
  for (auto &c : g.color) c = rng.below(C);
  g.cap.assign(C, K);
}

Instance random(Rng &rng, int N, int M, int C, int K) {
  Instance g;
  random_edges(rng, g, N, M);
  uniform_colors(rng, g, C, K);
  return g;
}

Instance grid(Rng &rng, int H, int W, int C, int K) {
  Instance g;
  g.n = H * W;
  for (int i = 0; i < H; ++i)
    for (int j = 0; j < W; ++j) {
      const int v = i * W + j;
      if (j + 1 < W) g.e.push_back({v, v + 1});
      if (i + 1 < H) g.e.push_back({v, v + W});
    }
  uniform_colors(rng, g, C, K);
  return g;
}

Instance vertex(Rng &rng, int N, int M, int K) {
  Instance g;
  random_edges(rng, g, N, M);
  g.color.resize(g.e.size());
  for (size_t i = 0; i < g.e.size(); ++i) g.color[i] = g.e[i][rng.below(2)];
  g.cap.assign(N, K);
  return g;
}

Instance few(Rng &rng, int N, int M, int C) {
  Instance g;
  random_edges(rng, g, N, M);
  g.color.resize(g.e.size());
  for (auto &c : g.color) c = rng.below(C);
  // N - 1 を C 個の正の数に分ける (区切りの位置を C - 1 個選ぶ)
  const int total = max(N - 1, C);
  vector<int> cut;
  for (int k = 0; k < C - 1; ++k) cut.push_back(rng.range(1, total - 1));
  sort(cut.begin(), cut.end());
  cut.insert(cut.begin(), 0), cut.push_back(total);
  g.cap.resize(C);
  for (int k = 0; k < C; ++k) g.cap[k] = max(1, cut[k + 1] - cut[k]);
  return g;
}

Instance complete(Rng &rng, int N) {
  Instance g;
  g.n = N;
  for (int u = 0; u < N; ++u)
    for (int v = u + 1; v < N; ++v) g.e.push_back({u, v});
  uniform_colors(rng, g, max(1, N - 1), 1);
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
  gen::Instance g;
  long long wt = 1;
  if (fam == "random") { auto a = args(5); g = gen::random(rng, a[0], a[1], a[2], a[3]), wt = a[4]; }
  else if (fam == "grid") { auto a = args(5); g = gen::grid(rng, a[0], a[1], a[2], a[3]), wt = a[4]; }
  else if (fam == "vertex") { auto a = args(4); g = gen::vertex(rng, a[0], a[1], a[2]), wt = a[3]; }
  else if (fam == "few") { auto a = args(4); g = gen::few(rng, a[0], a[1], a[2]), wt = a[3]; }
  else if (fam == "complete") { auto a = args(2); g = gen::complete(rng, a[0]), wt = a[1]; }
  else {
    fprintf(stderr, "unknown family %s\n", fam.c_str());
    return 1;
  }
  g.w.resize(g.e.size());
  for (auto &x : g.w) x = 1 + rng.below((u64)wt);
  // 頂点の番号、辺の順、各辺の向きを並べ替える (色は辺と一緒に動かす)
  {
    vector<int> p(g.n);
    iota(p.begin(), p.end(), 0);
    for (int i = g.n; i > 1; --i) swap(p[i - 1], p[rng.below(i)]);
    for (auto &e : g.e) e = {p[e[0]], p[e[1]]};
    for (size_t i = g.e.size(); i > 1; --i) {
      const int j = rng.below(i);
      swap(g.e[i - 1], g.e[j]), swap(g.color[i - 1], g.color[j]), swap(g.w[i - 1], g.w[j]);
    }
    for (auto &e : g.e)
      if (rng.next() >> 63) swap(e[0], e[1]);
  }

  auto t0 = chrono::steady_clock::now();
  Solver sol(g.n, g.e, g.color, g.cap, g.w);
  sol.run();
  auto t1 = chrono::steady_clock::now();

  const vector<i64> best = sol.answer();
  const vector<int> ans = sol.chosen();
  const int m = g.e.size();
  vector<char> used(m);
  vector<int> par(g.n), cnt(g.cap.size());
  iota(par.begin(), par.end(), 0);
  auto find = [&](int x) {
    while (par[x] != x) x = par[x] = par[par[x]];
    return x;
  };
  string bad;
  i64 total = 0;
  for (int i : ans) {
    if (i < 0 || i >= m) { bad = "edge id out of range"; break; }
    if (used[i]) { bad = "edge used twice"; break; }
    used[i] = 1;
    const int a = find(g.e[i][0]), b = find(g.e[i][1]);
    if (a == b) { bad = "cycle"; break; }
    par[a] = b;
    if (++cnt[g.color[i]] > g.cap[g.color[i]]) { bad = "color over capacity"; break; }
    total += g.w[i];
  }
  if (bad.empty() && best.empty()) bad = "empty answer";
  if (bad.empty() && ans.size() + 1 != best.size()) bad = "chosen size differs from answer().size() - 1";
  if (bad.empty() && total != best.back()) bad = "chosen weight differs from answer()[r]";
  if (bad.empty()) {
    string out = to_string(best.size() - 1) + "\n";
    for (size_t k = 0; k < best.size(); ++k) out += to_string(best[k]) + (k + 1 < best.size() ? " " : "\n");
    fputs(out.c_str(), stdout);
  } else printf("invalid: %s\n", bad.c_str());

  report_metrics((long long)chrono::duration_cast<chrono::nanoseconds>(t1 - t0).count());
}
