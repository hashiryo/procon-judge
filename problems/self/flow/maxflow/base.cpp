// 自作の問題。最大流の値を、性質の違うグラフの族で測る。
//
// 提出は次を実装する (loj-127 と同じ)。
//   struct Solver {
//     Solver(int n, int s, int t, const vector<array<int, 3>> &edges);  // {u, v, 容量}
//     void run();         // s から t への最大流を求める
//     i64 answer() const;  // 最大流の値
//   };
//
// 入力は 1 行で、グラフの族の名前、seed、引数を並べる。グラフはハーネスが計測の前に作る。容量はどれも 1 以上
// 2^30 以下で、辺は有向。構築と run を測る。edges は計測区間のあとまで生きているので、提出は参照を持ってよい。
//
//   grid H W C D       H × W のマス目。隣り合うマスの間に向きごとに容量 [1, C] の辺を張り、各マスで [-D, D] の値を
//                      引いて、正なら s からそのマスへ、負ならそのマスから t へ、その大きさの辺を張る (画像の領域分割
//                      の形)。マスの番号は行優先で、辺もマスの順に並ぶ
//   bipartite L R D    単位容量の二部グラフ。s から左の各頂点へ、左の各頂点から乱択の右 D 個へ、右の各頂点から t へ
//                      容量 1 の辺
//   band N D1 D2       左の i から右の i - D1 + 1 .. i へ、さらに右の [0, i] から乱択の D2 個へ辺を張った二部グラフを、
//                      単位容量の流れにしたもの (Library Checker の bipartitematching の unique_matching と同じ形)。
//                      頂点の番号と辺の順は乱択で並べ替える
//   sparse N M K C     N 頂点に乱択の辺を M 本 (容量 [1, C])。s から乱択の K 頂点へ、乱択の K 頂点から t へ容量 C の辺
//   genrmf A B C1 C2   A × A の格子 (frame) を B 枚並べる (Goldfarb と Grigoriadis の RMF)。frame の中は隣り合う頂点の
//                      間に向きごとに容量 C2 A^2 の辺、frame z の各頂点から frame z + 1 の乱択の置換の行き先へ容量
//                      [C1, C2] の辺。s は最初の frame の角、t は最後の frame の反対の角
//   rlg L W D C        幅 W の層を L 段並べ、各頂点から次の段の乱択の D 頂点へ容量 [1, C] の辺 (Washington の random
//                      level graph)。s から最初の段の各頂点へ、最後の段の各頂点から t へ容量 C D の辺
//   closure P Q D C    project selection の形。s から P 個の仕事へ容量 [1, C]、各仕事から乱択の D 個の道具へ容量 2^30、
//                      道具から t へ容量 [1, C]
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
  int n = 0, s = 0, t = 0;
  vector<array<int, 3>> e;
};

constexpr int BIG = 1 << 30;

Graph grid(Rng &rng, int H, int W, int C, int D) {
  Graph g;
  g.n = H * W + 2, g.s = H * W, g.t = H * W + 1;
  g.e.reserve((size_t)H * W * 5);
  for (int i = 0; i < H; ++i)
    for (int j = 0; j < W; ++j) {
      const int v = i * W + j;
      if (j + 1 < W) g.e.push_back({v, v + 1, rng.range(1, C)}), g.e.push_back({v + 1, v, rng.range(1, C)});
      if (i + 1 < H) g.e.push_back({v, v + W, rng.range(1, C)}), g.e.push_back({v + W, v, rng.range(1, C)});
      const int w = rng.range(-D, D);
      if (w > 0) g.e.push_back({g.s, v, w});
      if (w < 0) g.e.push_back({v, g.t, -w});
    }
  return g;
}

Graph bipartite(Rng &rng, int L, int R, int D) {
  Graph g;
  g.n = L + R + 2, g.s = L + R, g.t = L + R + 1;
  for (int a = 0; a < L; ++a) {
    g.e.push_back({g.s, a, 1});
    for (int k = 0; k < D; ++k) g.e.push_back({a, L + rng.below(R), 1});
  }
  for (int b = 0; b < R; ++b) g.e.push_back({L + b, g.t, 1});
  return g;
}

Graph band(Rng &rng, int N, int D1, int D2) {
  vector<int> pl(N), pr(N);
  iota(pl.begin(), pl.end(), 0), iota(pr.begin(), pr.end(), 0);
  for (int i = N; i > 1; --i) swap(pl[i - 1], pl[rng.below(i)]);
  for (int i = N; i > 1; --i) swap(pr[i - 1], pr[rng.below(i)]);
  Graph g;
  g.n = 2 * N + 2, g.s = 2 * N, g.t = 2 * N + 1;
  for (int i = 0; i < N; ++i) {
    for (int j = max(0, i - D1 + 1); j <= i; ++j) g.e.push_back({pl[i], N + pr[j], 1});
    for (int k = 0; k < D2; ++k) g.e.push_back({pl[i], N + pr[rng.below(i + 1)], 1});
  }
  for (int i = 0; i < N; ++i) g.e.push_back({g.s, i, 1}), g.e.push_back({N + i, g.t, 1});
  for (size_t i = g.e.size(); i > 1; --i) swap(g.e[i - 1], g.e[rng.below(i)]);
  return g;
}

Graph sparse(Rng &rng, int N, int M, int K, int C) {
  Graph g;
  g.n = N + 2, g.s = N, g.t = N + 1;
  for (int i = 0; i < M; ++i) {
    const int u = rng.below(N), v = rng.below(N);
    g.e.push_back({u, v, rng.range(1, C)});
  }
  for (int i = 0; i < K; ++i) g.e.push_back({g.s, rng.below(N), C});
  for (int i = 0; i < K; ++i) g.e.push_back({rng.below(N), g.t, C});
  return g;
}

Graph genrmf(Rng &rng, int A, int B, int C1, int C2) {
  Graph g;
  const int F = A * A;
  g.n = F * B, g.s = 0, g.t = F * B - 1;
  const int in = C2 * F;
  vector<int> perm(F);
  for (int z = 0; z < B; ++z) {
    for (int x = 0; x < A; ++x)
      for (int y = 0; y < A; ++y) {
        const int v = z * F + x * A + y;
        if (y + 1 < A) g.e.push_back({v, v + 1, in}), g.e.push_back({v + 1, v, in});
        if (x + 1 < A) g.e.push_back({v, v + A, in}), g.e.push_back({v + A, v, in});
      }
    if (z + 1 < B) {
      iota(perm.begin(), perm.end(), 0);
      for (int i = F; i > 1; --i) swap(perm[i - 1], perm[rng.below(i)]);
      for (int i = 0; i < F; ++i) g.e.push_back({z * F + i, (z + 1) * F + perm[i], rng.range(C1, C2)});
    }
  }
  return g;
}

Graph rlg(Rng &rng, int L, int W, int D, int C) {
  Graph g;
  g.n = L * W + 2, g.s = L * W, g.t = L * W + 1;
  for (int i = 0; i < W; ++i) g.e.push_back({g.s, i, C * D});
  for (int l = 0; l + 1 < L; ++l)
    for (int i = 0; i < W; ++i)
      for (int k = 0; k < D; ++k) g.e.push_back({l * W + i, (l + 1) * W + rng.below(W), rng.range(1, C)});
  for (int i = 0; i < W; ++i) g.e.push_back({(L - 1) * W + i, g.t, C * D});
  return g;
}

Graph closure(Rng &rng, int P, int Q, int D, int C) {
  Graph g;
  g.n = P + Q + 2, g.s = P + Q, g.t = P + Q + 1;
  for (int a = 0; a < P; ++a) {
    g.e.push_back({g.s, a, rng.range(1, C)});
    for (int k = 0; k < D; ++k) g.e.push_back({a, P + rng.below(Q), BIG});
  }
  for (int b = 0; b < Q; ++b) g.e.push_back({P + b, g.t, rng.range(1, C)});
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
  if (fam == "grid") { auto a = args(4); g = gen::grid(rng, a[0], a[1], a[2], a[3]); }
  else if (fam == "bipartite") { auto a = args(3); g = gen::bipartite(rng, a[0], a[1], a[2]); }
  else if (fam == "band") { auto a = args(3); g = gen::band(rng, a[0], a[1], a[2]); }
  else if (fam == "sparse") { auto a = args(4); g = gen::sparse(rng, a[0], a[1], a[2], a[3]); }
  else if (fam == "genrmf") { auto a = args(4); g = gen::genrmf(rng, a[0], a[1], a[2], a[3]); }
  else if (fam == "rlg") { auto a = args(4); g = gen::rlg(rng, a[0], a[1], a[2], a[3]); }
  else if (fam == "closure") { auto a = args(4); g = gen::closure(rng, a[0], a[1], a[2], a[3]); }
  else {
    fprintf(stderr, "unknown family %s\n", fam.c_str());
    return 1;
  }

  auto t0 = chrono::steady_clock::now();
  Solver sol(g.n, g.s, g.t, g.e);
  sol.run();
  auto t1 = chrono::steady_clock::now();

  printf("%lld\n", sol.answer());

  report_metrics(
      (long long)chrono::duration_cast<chrono::nanoseconds>(t1 - t0).count());
}
