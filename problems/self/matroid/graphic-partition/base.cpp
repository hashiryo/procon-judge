// 自作の問題。グラフ的マトロイドと分割マトロイドの最大共通独立集合 (重みなしのマトロイド交差) を、性質の違う族で測る。
// 辺に色があり、色 c の辺は cap[c] 本まで使える。閉路を作らない辺の集合のうち、要素数が最大のものを求める。
//
// 提出は次を実装する。
//   struct Solver {
//     Solver(int n, const vector<array<int, 2>> &edges, const vector<int> &color, const vector<int> &cap);
//     void run();                  // 最大共通独立集合を求める
//     vector<int> answer() const;  // 選んだ辺の番号
//   };
// n は頂点数。辺 {u, v} は u != v で、多重辺はありうる。color[i] は辺 i の色で 0 以上 cap.size() 未満、cap[c] >= 0。
//
// 入力は 1 行で、族の名前、seed、引数を並べる。グラフはハーネスが計測の前に作り、頂点の番号、辺の順、各辺の向きを乱択で
// 並べ替える。構築と run を測る。answer は計測区間の外で呼び、ハーネスが辺の番号の範囲と重複、閉路が無いこと、色ごとの
// 本数を確かめてから、要素数を出す (崩れていれば invalid)。引数の参照は計測区間のあとまで生きているので、提出は参照を
// 持ってよい。
//
//   random N M C K   N 頂点に乱択の辺を M 本。色は C 色から一様に選び、どの色も上限 K
//   grid H W C K     H × W の格子の辺。色は C 色から一様に選び、どの色も上限 K
//   vertex N M K     N 頂点に乱択の辺を M 本。辺を乱択の端点に割り当て、その頂点を色とする (頂点ごとに K 本まで)
//   few N M C        N 頂点に乱択の辺を M 本。色は C 色から一様に選び、上限は N - 1 を C 個の正の数に乱択で分けたもの
//   complete N       N 頂点の完全グラフ。色は N - 1 色から一様に選び、どの色も上限 1 (虹色の全域木)
//   cycles N L C     長さ L の閉路を N / L 個並べ、隣り合う閉路を 1 本ずつの辺で結ぶ。色は C 色から一様に選び、どの色も
//                    上限 1。短い閉路ごとに 1 本ずつ外す色の選び方が絡み合い、増加路が長くなりやすい
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

Instance cycles(Rng &rng, int N, int L, int C) {
  Instance g;
  const int k = max(1, N / L);
  g.n = k * L;
  for (int b = 0; b < k; ++b) {
    for (int i = 0; i < L; ++i) g.e.push_back({b * L + i, b * L + (i + 1) % L});
    if (b + 1 < k) g.e.push_back({b * L + rng.below(L), (b + 1) * L + rng.below(L)});
  }
  uniform_colors(rng, g, C, 1);
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
  if (fam == "random") { auto a = args(4); g = gen::random(rng, a[0], a[1], a[2], a[3]); }
  else if (fam == "grid") { auto a = args(4); g = gen::grid(rng, a[0], a[1], a[2], a[3]); }
  else if (fam == "vertex") { auto a = args(3); g = gen::vertex(rng, a[0], a[1], a[2]); }
  else if (fam == "few") { auto a = args(3); g = gen::few(rng, a[0], a[1], a[2]); }
  else if (fam == "complete") { auto a = args(1); g = gen::complete(rng, a[0]); }
  else if (fam == "cycles") { auto a = args(3); g = gen::cycles(rng, a[0], a[1], a[2]); }
  else {
    fprintf(stderr, "unknown family %s\n", fam.c_str());
    return 1;
  }
  // 頂点の番号、辺の順、各辺の向きを並べ替える (色は辺と一緒に動かす)
  {
    vector<int> p(g.n);
    iota(p.begin(), p.end(), 0);
    for (int i = g.n; i > 1; --i) swap(p[i - 1], p[rng.below(i)]);
    for (auto &e : g.e) e = {p[e[0]], p[e[1]]};
    for (size_t i = g.e.size(); i > 1; --i) {
      const int j = rng.below(i);
      swap(g.e[i - 1], g.e[j]), swap(g.color[i - 1], g.color[j]);
    }
    for (auto &e : g.e)
      if (rng.next() >> 63) swap(e[0], e[1]);
  }

  auto t0 = chrono::steady_clock::now();
  Solver sol(g.n, g.e, g.color, g.cap);
  sol.run();
  auto t1 = chrono::steady_clock::now();

  const vector<int> ans = sol.answer();
  const int m = g.e.size();
  vector<char> used(m);
  vector<int> par(g.n), cnt(g.cap.size());
  iota(par.begin(), par.end(), 0);
  auto find = [&](int x) {
    while (par[x] != x) x = par[x] = par[par[x]];
    return x;
  };
  string bad;
  for (int i : ans) {
    if (i < 0 || i >= m) { bad = "edge id out of range"; break; }
    if (used[i]) { bad = "edge used twice"; break; }
    used[i] = 1;
    const int a = find(g.e[i][0]), b = find(g.e[i][1]);
    if (a == b) { bad = "cycle"; break; }
    par[a] = b;
    if (++cnt[g.color[i]] > g.cap[g.color[i]]) { bad = "color over capacity"; break; }
  }
  if (bad.empty()) printf("%d\n", (int)ans.size());
  else printf("invalid: %s\n", bad.c_str());

  report_metrics((long long)chrono::duration_cast<chrono::nanoseconds>(t1 - t0).count());
}
