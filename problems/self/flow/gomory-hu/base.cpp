// 自作の問題。無向グラフの Gomory-Hu 木 (全点対の最小カットを表す木) を、性質の違うグラフの族で作る。
//
// 提出は次を実装する (loj-2042、luogu-P4897、luogu-P3329 と同じ)。
//   struct Solver {
//     Solver(int n, const vector<array<i64, 3>> &edges);  // 無向辺 {u, v, 容量}。頂点は 0 から n - 1
//     void run();                                        // Gomory-Hu 木を作る
//     const vector<array<i64, 3>> &tree() const;         // 木の辺 {u, v, 重み} を n - 1 本
//   };
//
// Gomory-Hu 木は、同じ頂点の上の重み付きの木で、木の辺 e を切って分かれる頂点集合のカットの容量が e の重みに等しく、
// どの 2 頂点 s, t についても、木の s-t の道の上の重みの最小が s-t の最小カットの値 λ(s, t) に等しいもの。
//
// 入力は 1 行で、グラフの族の名前、seed、引数を並べる。グラフはハーネスが計測の前に作る。容量は 0 以上 2^30 以下で、
// 多重辺はあるが自己ループは無い。構築と run を測る。edges は計測区間のあとまで生きているので、提出は参照を持ってよい。
//
// 計測のあとに次を確かめ、崩れていれば "invalid" と理由を出す (期待出力と合わないので WA になる)。
//   1. tree() が n - 1 本の辺で、全頂点をつなぐ木であること。
//   2. 木の辺ごとに、その辺で切った頂点集合のカットの容量 (元のグラフで端が両側に分かれる辺の容量の和) が辺の重みに
//      等しいこと。木を頂点 0 で根付け、元の辺 (a, b, c) ごとに a と b に c を、lca(a, b) に -2c を足して部分木の和を取る。
// 2 が成り立てば、どの組でも道の上の最小の辺で切ったカットが s と t を分けるので、道の上の最小は λ(s, t) 以上になる。
// そこで、全組の道の上の最小の和 S (重みの大きい順に Union-Find でつなぎ、つないだ 2 つの成分の大きさの積に重みを
// 掛けて足す) を出す。S が参照実装の値 (= λ の和) と等しければ、すべての組で道の上の最小が λ に等しい。
//
//   random N M C0 C1    N 頂点に乱択の辺 M 本 (両端は異なる)、容量 [C0, C1]。M が小さいと連結とは限らない
//   dense N C           完全グラフ K_N、容量 [1, C]
//   grid H W C          H × W のマス目、隣り合うマスの間に容量 [1, C] の辺
//   cluster K S D X C   S 頂点の塊を K 個。各頂点から同じ塊の乱択の頂点へ D 本、塊をまたぐ乱択の辺を X 本。容量は
//                       どれも [1, C]。塊の境目に、頂点 1 つを切り離すのでない最小カットができる
//   unit N M            N 頂点に乱択の辺 M 本、容量 1 (λ の値の重なりが多い)
//   cycle N K C         長さ N の輪 (容量 [C, 2C]) に、乱択の弦を K 本 (容量 [1, C])。輪の 2 本の辺で切るカットが多い
//   tree N K C          乱択の木 (頂点 i の親は [0, i - 1] の乱択、容量 [1, C]) に、乱択の辺を K 本 (容量 [1, C])
//   hub N H M C         H 個の中心から残りの全頂点へ辺 (容量 [1, C])、中心でない頂点の間に乱択の辺 M 本 (容量 [1, C])
//   comps N K M C       N 頂点を K 個の成分に分け、成分の中に乱択の辺を合わせて M 本 (容量 [1, C])。成分をまたぐ組は λ = 0
//   path N C            長さ N の道、容量 [0, C]
// どの族も、頂点の番号と辺の順を乱択で並べ替える。
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
  i64 below(u64 n) { return (i64)((unsigned __int128)next() * n >> 64); }
  // [lo, hi]
  i64 range(i64 lo, i64 hi) { return lo + below((u64)(hi - lo) + 1); }
};

struct Graph {
  int n = 0;
  vector<array<i64, 3>> e;
  void add(int u, int v, i64 c) { e.push_back({u, v, c}); }
};

template <class T> void shuffle(Rng &rng, vector<T> &v) {
  for (size_t i = v.size(); i > 1; --i) swap(v[i - 1], v[rng.below(i)]);
}

// 頂点の番号と辺の順、辺の向きを乱択で並べ替える。
void relabel(Rng &rng, Graph &g) {
  vector<int> id(g.n);
  iota(id.begin(), id.end(), 0);
  shuffle(rng, id);
  for (auto &e : g.e) {
    e[0] = id[e[0]], e[1] = id[e[1]];
    if (rng.below(2)) swap(e[0], e[1]);
  }
  shuffle(rng, g.e);
}

// [lo, hi) の中で u と異なる頂点。
int other(Rng &rng, int lo, int hi, int u) {
  int v = (int)rng.range(lo, hi - 2);
  return v >= u ? v + 1 : v;
}

Graph random_graph(Rng &rng, int N, int M, i64 C0, i64 C1) {
  Graph g;
  g.n = N;
  if (N >= 2)
    for (int i = 0; i < M; ++i) {
      const int u = (int)rng.below(N);
      g.add(u, other(rng, 0, N, u), rng.range(C0, C1));
    }
  return g;
}

Graph dense(Rng &rng, int N, i64 C) {
  Graph g;
  g.n = N;
  for (int u = 0; u < N; ++u)
    for (int v = u + 1; v < N; ++v) g.add(u, v, rng.range(1, C));
  return g;
}

Graph grid(Rng &rng, int H, int W, i64 C) {
  Graph g;
  g.n = H * W;
  for (int i = 0; i < H; ++i)
    for (int j = 0; j < W; ++j) {
      const int v = i * W + j;
      if (j + 1 < W) g.add(v, v + 1, rng.range(1, C));
      if (i + 1 < H) g.add(v, v + W, rng.range(1, C));
    }
  return g;
}

Graph cluster(Rng &rng, int K, int S, int D, int X, i64 C) {
  Graph g;
  g.n = K * S;
  if (S >= 2)
    for (int v = 0; v < g.n; ++v) {
      const int lo = v / S * S;
      for (int d = 0; d < D; ++d) g.add(v, other(rng, lo, lo + S, v), rng.range(1, C));
    }
  if (K >= 2)
    for (int i = 0; i < X; ++i) {
      const int a = (int)rng.below(K), b = other(rng, 0, K, a);
      g.add(a * S + (int)rng.below(S), b * S + (int)rng.below(S), rng.range(1, C));
    }
  return g;
}

Graph cycle(Rng &rng, int N, int K, i64 C) {
  Graph g;
  g.n = N;
  if (N >= 2) {
    for (int v = 0; v < N; ++v)
      if (N > 2 || v == 0) g.add(v, (v + 1) % N, rng.range(C, 2 * C));
    for (int i = 0; i < K; ++i) {
      const int u = (int)rng.below(N);
      g.add(u, other(rng, 0, N, u), rng.range(1, C));
    }
  }
  return g;
}

Graph tree_plus(Rng &rng, int N, int K, i64 C) {
  Graph g;
  g.n = N;
  for (int v = 1; v < N; ++v) g.add(v, (int)rng.below(v), rng.range(1, C));
  if (N >= 2)
    for (int i = 0; i < K; ++i) {
      const int u = (int)rng.below(N);
      g.add(u, other(rng, 0, N, u), rng.range(1, C));
    }
  return g;
}

Graph hub(Rng &rng, int N, int H, int M, i64 C) {
  Graph g;
  g.n = N;
  H = min(H, N);
  for (int h = 0; h < H; ++h)
    for (int v = H; v < N; ++v) g.add(h, v, rng.range(1, C));
  if (N - H >= 2)
    for (int i = 0; i < M; ++i) {
      const int u = H + (int)rng.below(N - H);
      g.add(u, other(rng, H, N, u), rng.range(1, C));
    }
  return g;
}

Graph comps(Rng &rng, int N, int K, int M, i64 C) {
  Graph g;
  g.n = N;
  K = max(1, min(K, N));
  for (int i = 0; i < M; ++i) {
    const int k = (int)rng.below(K), lo = (int)((i64)N * k / K), hi = (int)((i64)N * (k + 1) / K);
    if (hi - lo < 2) continue;
    const int u = lo + (int)rng.below(hi - lo);
    g.add(u, other(rng, lo, hi, u), rng.range(1, C));
  }
  return g;
}

Graph path(Rng &rng, int N, i64 C) {
  Graph g;
  g.n = N;
  for (int v = 0; v + 1 < N; ++v) g.add(v, v + 1, rng.range(0, C));
  return g;
}
}  // namespace gen

static string i128_to_string(__int128 v) {
  if (v == 0) return "0";
  const bool neg = v < 0;
  unsigned __int128 x = neg ? -(unsigned __int128)v : (unsigned __int128)v;
  string s;
  while (x) s += char('0' + (int)(x % 10)), x /= 10;
  if (neg) s += '-';
  return string(s.rbegin(), s.rend());
}

// 提出の木を確かめ、全組の道の上の最小の和を S に入れる。崩れていれば理由を返す。
static string verify(const gen::Graph &g, const vector<array<i64, 3>> &t, __int128 &S) {
  const int n = g.n;
  if ((int)t.size() != max(n - 1, 0)) return "tree() の辺の数が n - 1 でない";
  vector<vector<pair<int, i64>>> adj(n);
  for (auto &[u, v, w] : t) {
    if (u < 0 || u >= n || v < 0 || v >= n || u == v) return "木の辺の端が範囲の外か、自己ループ";
    if (w < 0) return "木の辺の重みが負";
    adj[u].push_back({(int)v, w}), adj[v].push_back({(int)u, w});
  }
  vector<int> par(n, -1), order, dep(n, 0);
  vector<i64> pw(n, 0);
  if (n > 0) {
    vector<char> seen(n, 0);
    seen[0] = 1, order.push_back(0);
    for (size_t i = 0; i < order.size(); ++i) {
      const int u = order[i];
      for (auto [v, w] : adj[u])
        if (!seen[v]) seen[v] = 1, par[v] = u, pw[v] = w, dep[v] = dep[u] + 1, order.push_back(v);
    }
    if ((int)order.size() != n) return "tree() が全頂点をつながない";
  }
  int lg = 1;
  while ((1 << lg) < n) ++lg;
  vector<vector<int>> up(lg, vector<int>(n));
  for (int v = 0; v < n; ++v) up[0][v] = par[v] < 0 ? v : par[v];
  for (int k = 1; k < lg; ++k)
    for (int v = 0; v < n; ++v) up[k][v] = up[k - 1][up[k - 1][v]];
  auto lca = [&](int a, int b) {
    if (dep[a] < dep[b]) swap(a, b);
    for (int k = lg - 1; k >= 0; --k)
      if (dep[a] - (1 << k) >= dep[b]) a = up[k][a];
    if (a == b) return a;
    for (int k = lg - 1; k >= 0; --k)
      if (up[k][a] != up[k][b]) a = up[k][a], b = up[k][b];
    return up[0][a];
  };
  vector<i64> d(n, 0);
  for (auto &[a, b, c] : g.e) {
    if (a == b) continue;
    d[a] += c, d[b] += c, d[lca((int)a, (int)b)] -= 2 * c;
  }
  for (int i = n - 1; i >= 1; --i) {
    const int v = order[i];
    if (d[v] != pw[v]) return "木の辺 (" + to_string(v) + ", " + to_string(par[v]) + ") の重みが、切った頂点集合のカットの容量と違う";
    d[par[v]] += d[v];
  }
  vector<array<i64, 3>> es(t);
  sort(es.begin(), es.end(), [](auto &x, auto &y) { return x[2] > y[2]; });
  vector<int> uf(n, -1);
  auto find = [&](int v) {
    while (uf[v] >= 0) {
      if (uf[uf[v]] >= 0) uf[v] = uf[uf[v]];
      v = uf[v];
    }
    return v;
  };
  S = 0;
  for (auto &[u, v, w] : es) {
    int a = find((int)u), b = find((int)v);
    S += (__int128)w * (-uf[a]) * (-uf[b]);
    if (uf[a] > uf[b]) swap(a, b);
    uf[a] += uf[b], uf[b] = a;
  }
  return "";
}

signed main() {
  const string fam = read_token();
  long long seed;
  must_scan(scanf("%lld", &seed), 1);
  gen::Rng rng{(u64)seed};
  auto args = [&](int k) {
    vector<i64> a(k);
    for (auto &x : a) must_scan(scanf("%lld", &x), 1);
    return a;
  };
  gen::Graph g;
  if (fam == "random") { auto a = args(4); g = gen::random_graph(rng, a[0], a[1], a[2], a[3]); }
  else if (fam == "dense") { auto a = args(2); g = gen::dense(rng, a[0], a[1]); }
  else if (fam == "grid") { auto a = args(3); g = gen::grid(rng, a[0], a[1], a[2]); }
  else if (fam == "cluster") { auto a = args(5); g = gen::cluster(rng, a[0], a[1], a[2], a[3], a[4]); }
  else if (fam == "unit") { auto a = args(2); g = gen::random_graph(rng, a[0], a[1], 1, 1); }
  else if (fam == "cycle") { auto a = args(3); g = gen::cycle(rng, a[0], a[1], a[2]); }
  else if (fam == "tree") { auto a = args(3); g = gen::tree_plus(rng, a[0], a[1], a[2]); }
  else if (fam == "hub") { auto a = args(4); g = gen::hub(rng, a[0], a[1], a[2], a[3]); }
  else if (fam == "comps") { auto a = args(4); g = gen::comps(rng, a[0], a[1], a[2], a[3]); }
  else if (fam == "path") { auto a = args(2); g = gen::path(rng, a[0], a[1]); }
  else {
    fprintf(stderr, "unknown family %s\n", fam.c_str());
    return 1;
  }
  gen::relabel(rng, g);

  auto t0 = chrono::steady_clock::now();
  Solver sol(g.n, g.e);
  sol.run();
  auto t1 = chrono::steady_clock::now();

  __int128 S = 0;
  if (string why = verify(g, sol.tree(), S); !why.empty()) printf("invalid: %s\n", why.c_str());
  else printf("%s\n", i128_to_string(S).c_str());

  report_metrics(
      (long long)chrono::duration_cast<chrono::nanoseconds>(t1 - t0).count());
}
