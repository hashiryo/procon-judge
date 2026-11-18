// 自作の問題。グラフ的マトロイドの和 (辺の集合を k 本の森に分けて覆う) を、性質の違う族で測る。辺に森の番号を
// 割り当て (使わない辺は -1)、同じ番号の辺が閉路を作らないようにして、割り当てた辺の数を最大にする。
//
// 提出は次を実装する。
//   struct Solver {
//     Solver(int n, const vector<array<int, 2>> &edges, int k);
//     void run();                  // 割り当てを求める
//     vector<int> answer() const;  // 辺ごとの森の番号 (0 以上 k 未満、使わない辺は -1)
//   };
// n は頂点数、k >= 1 は森の本数。辺 {u, v} は u != v で、多重辺はありうる。
//
// 入力は 1 行で、族の名前、seed、引数を並べる。グラフはハーネスが計測の前に作り、頂点の番号、辺の順、各辺の向きを乱択で
// 並べ替える。構築と run を測る。answer は計測区間の外で呼び、ハーネスが長さと番号の範囲、森ごとに閉路が無いことを
// 確かめてから、割り当てた辺の数を出す (崩れていれば invalid)。引数の参照は計測区間のあとまで生きているので、提出は
// 参照を持ってよい。
//
//   random N M K      N 頂点に乱択の辺を M 本
//   grid H W K        H × W の格子の辺 (K = 2 なら全部の辺を覆える)
//   cubic N K         N 頂点の乱択の 3 正則の多重グラフ (K = 2 なら全部の辺を覆える。TCO18 ColoringEdgesDiv1 の形)
//   complete N K      N 頂点の完全グラフ (N が偶数で K = N / 2 なら、ちょうど K 本の全域木に分けられる)
//   core N M T K      N 頂点に乱択の辺を M 本と、そのうち T 頂点の完全グラフ。完全グラフの部分の辺の多くは覆えない
//   planted N K R     N 頂点の一様な乱択の全域木を K 本と、乱択の辺を R 本 (K 本の全域木の辺は全部を覆える)
//   multi N M K       少ない N 頂点に乱択の辺を M 本。多重辺が多く、覆えない辺が多い
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
};

struct Instance {
  int n = 0, k = 1;
  vector<array<int, 2>> e;
};

void random_edges(Rng &rng, Instance &g, int M) {
  for (int c = 0; g.n >= 2 && c < M;) {
    const int u = rng.below(g.n), v = rng.below(g.n);
    if (u != v) g.e.push_back({u, v}), ++c;
  }
}

Instance random(Rng &rng, int N, int M, int K) {
  Instance g;
  g.n = N, g.k = K;
  random_edges(rng, g, M);
  return g;
}

Instance grid(int H, int W, int K) {
  Instance g;
  g.n = H * W, g.k = K;
  for (int i = 0; i < H; ++i)
    for (int j = 0; j < W; ++j) {
      const int v = i * W + j;
      if (j + 1 < W) g.e.push_back({v, v + 1});
      if (i + 1 < H) g.e.push_back({v, v + W});
    }
  return g;
}

// 頂点ごとに 3 本の口を並べて乱択に組にする。自己ループになった組は、別の乱択の組と相手を入れ替えて直す。
Instance cubic(Rng &rng, int N, int K) {
  Instance g;
  g.n = N + (N & 1), g.k = K;
  vector<int> s(3 * g.n);
  for (int i = 0; i < 3 * g.n; ++i) s[i] = i / 3;
  for (int i = (int)s.size(); i > 1; --i) swap(s[i - 1], s[rng.below(i)]);
  const int p = s.size() / 2;
  for (int t = 0; t < 100 * p; ++t) {
    bool ok = true;
    for (int i = 0; i < p; ++i)
      if (s[2 * i] == s[2 * i + 1]) {
        ok = false;
        const int q = rng.below(p);
        const int side = rng.below(2);
        swap(s[2 * i + 1], s[2 * q + side]);
      }
    if (ok) break;
  }
  for (int i = 0; i < p; ++i)
    if (s[2 * i] != s[2 * i + 1]) g.e.push_back({s[2 * i], s[2 * i + 1]});
  return g;
}

Instance complete(int N, int K) {
  Instance g;
  g.n = N, g.k = K;
  for (int u = 0; u < N; ++u)
    for (int v = u + 1; v < N; ++v) g.e.push_back({u, v});
  return g;
}

Instance core(Rng &rng, int N, int M, int T, int K) {
  Instance g;
  g.n = N, g.k = K;
  random_edges(rng, g, M);
  for (int u = 0; u < min(T, N); ++u)
    for (int v = u + 1; v < min(T, N); ++v) g.e.push_back({u, v});
  return g;
}

// 完全グラフの上の乱歩で、一様な乱択の全域木を作る (Aldous-Broder)。
void uniform_tree(Rng &rng, Instance &g) {
  vector<char> seen(g.n);
  int cur = rng.below(g.n), cnt = 1;
  seen[cur] = 1;
  while (cnt < g.n) {
    int nx = rng.below(g.n - 1);
    if (nx >= cur) ++nx;
    if (!seen[nx]) seen[nx] = 1, ++cnt, g.e.push_back({cur, nx});
    cur = nx;
  }
}

Instance planted(Rng &rng, int N, int K, int R) {
  Instance g;
  g.n = N, g.k = K;
  for (int t = 0; N >= 2 && t < K; ++t) uniform_tree(rng, g);
  random_edges(rng, g, R);
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
  if (fam == "random") { auto a = args(3); g = gen::random(rng, a[0], a[1], a[2]); }
  else if (fam == "grid") { auto a = args(3); g = gen::grid(a[0], a[1], a[2]); }
  else if (fam == "cubic") { auto a = args(2); g = gen::cubic(rng, a[0], a[1]); }
  else if (fam == "complete") { auto a = args(2); g = gen::complete(a[0], a[1]); }
  else if (fam == "core") { auto a = args(4); g = gen::core(rng, a[0], a[1], a[2], a[3]); }
  else if (fam == "planted") { auto a = args(3); g = gen::planted(rng, a[0], a[1], a[2]); }
  else if (fam == "multi") { auto a = args(3); g = gen::random(rng, a[0], a[1], a[2]); }
  else {
    fprintf(stderr, "unknown family %s\n", fam.c_str());
    return 1;
  }
  // 頂点の番号、辺の順、各辺の向きを並べ替える
  {
    vector<int> p(g.n);
    iota(p.begin(), p.end(), 0);
    for (int i = g.n; i > 1; --i) swap(p[i - 1], p[rng.below(i)]);
    for (auto &e : g.e) e = {p[e[0]], p[e[1]]};
    for (size_t i = g.e.size(); i > 1; --i) swap(g.e[i - 1], g.e[rng.below(i)]);
    for (auto &e : g.e)
      if (rng.next() >> 63) swap(e[0], e[1]);
  }

  auto t0 = chrono::steady_clock::now();
  Solver sol(g.n, g.e, g.k);
  sol.run();
  auto t1 = chrono::steady_clock::now();

  const vector<int> ans = sol.answer();
  const int m = g.e.size();
  string bad;
  int cnt = 0;
  if ((int)ans.size() != m) bad = "answer length";
  else {
    vector<int> par((size_t)g.n * g.k);
    iota(par.begin(), par.end(), 0);
    auto find = [&](int x) {
      while (par[x] != x) x = par[x] = par[par[x]];
      return x;
    };
    for (int i = 0; i < m && bad.empty(); ++i) {
      if (ans[i] < -1 || ans[i] >= g.k) { bad = "forest id out of range"; break; }
      if (ans[i] < 0) continue;
      const int off = ans[i] * g.n, a = find(off + g.e[i][0]), b = find(off + g.e[i][1]);
      if (a == b) { bad = "cycle"; break; }
      par[a] = b, ++cnt;
    }
  }
  if (bad.empty()) printf("%d\n", cnt);
  else printf("invalid: %s\n", bad.c_str());

  report_metrics((long long)chrono::duration_cast<chrono::nanoseconds>(t1 - t0).count());
}
