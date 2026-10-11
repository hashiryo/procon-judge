// 自作の問題。最大流の値を、競プロで想定解は別にあるが網を作って最大流で解くこともできる問題の形で測る
// (algo-notes の network-flow-brute-force.md)。self-flow-maxflow と違い、辺は逆向きの容量も持つ (無向の辺を 1 組の弧で表せる)。
//
// 提出は次を実装する。
//   struct Solver {
//     Solver(int n, int s, int t, const vector<array<int, 4>> &edges);  // {u, v, 容量, 逆向きの容量}
//     void run();          // s から t への最大流を求める
//     i64 answer() const;  // 最大流の値
//   };
//
// 入力は 1 行で、グラフの族の名前、seed、引数を並べる。グラフはハーネスが計測の前に作る。容量は 0 以上 2^30 以下で、
// 容量と逆向きの容量の和は 2^31 未満 (残余容量を int で持てる)。
// 構築と run を測る。edges は計測区間のあとまで生きているので、提出は参照を持ってよい。
//
//   cgrid H W C          H × W の格子 (洛谷 P4001 狼抓兔子の形)。右、下、右下の隣と無向の辺 (容量 [1, C]) で結び、
//                        s は左上、t は右下。頂点は行優先で、辺も頂点の順に並ぶ
//   cgrid_dir H W C      (H + 1) × (W + 1) の格子点 (洛谷 P2046 NOI2010 海拔の形)。右と下の隣と、向きごとに容量の違う
//                        辺 (どちらも [1, C]) で結び、s は左上、t は右下。s と t につながる辺だけは容量を 100 C にして、角を
//                        切るだけの自明なカットにならないようにする
//   presuf N M           N 人と M 脚の椅子 (ARC 076 F の形)。人 i は L_i 以下か R_i 以上の椅子に座れる (L_i < R_i は
//                        乱択)。s → 人 (容量 1)、人 → 前の鎖の L_i と後ろの鎖の R_i (容量 N)、前の鎖 k → k - 1 と k → 椅子 k、
//                        後ろの鎖 k → k + 1 と k → 椅子 k (容量 N)、椅子 → t (容量 1)
//   segtree N M          N 人と M 脚の椅子。人 i は区間 [l_i, r_i] の椅子に座れる。椅子の上のセグメント木の節で区間を
//                        まとめ、人 → 区間を覆う節 (容量 N)、節 → 子 (容量 N)、葉 (椅子) → t (容量 1)、s → 人 (容量 1)。
//                        区間の長さは [1, M / 8] の乱択
//   closure P Q D C      頂点の少ない密な閉包 (洛谷 P4177 CEOI order の形)。s → 仕事 (容量は報酬 [1, D C])、仕事 → 乱択の
//                        D 台の機械 (容量は借りる費用 [1, C])、機械 → t (容量は買う費用 [1, P D C / Q])。報酬を借りる費用の和と、
//                        買う費用をその機械を使う仕事の数ぶんの借りる費用と同じくらいにして、答えが自明にならないようにする
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
  int n = 0, s = 0, t = 0;
  vector<array<int, 4>> e;
};

Graph cgrid(Rng &rng, int H, int W, int C) {
  Graph g;
  g.n = H * W, g.s = 0, g.t = H * W - 1;
  for (int i = 0; i < H; ++i)
    for (int j = 0; j < W; ++j) {
      const int v = i * W + j;
      if (j + 1 < W) { const int c = (int)rng.range(1, C); g.e.push_back({v, v + 1, c, c}); }
      if (i + 1 < H) { const int c = (int)rng.range(1, C); g.e.push_back({v, v + W, c, c}); }
      if (i + 1 < H && j + 1 < W) { const int c = (int)rng.range(1, C); g.e.push_back({v, v + W + 1, c, c}); }
    }
  return g;
}

Graph cgrid_dir(Rng &rng, int H, int W, int C) {
  Graph g;
  const int w = W + 1;
  g.n = (H + 1) * w, g.s = 0, g.t = g.n - 1;
  for (int i = 0; i <= H; ++i)
    for (int j = 0; j <= W; ++j) {
      const int v = i * w + j;
      const bool end = (v == g.s);
      if (j < W) g.e.push_back({v, v + 1, (int)rng.range(1, C) * (end || v + 1 == g.t ? 100 : 1), (int)rng.range(1, C)});
      if (i < H) g.e.push_back({v, v + w, (int)rng.range(1, C) * (end || v + w == g.t ? 100 : 1), (int)rng.range(1, C)});
    }
  return g;
}

Graph presuf(Rng &rng, int N, int M) {
  // 頂点: 人 0..N-1、前の鎖 N..N+M-1、後ろの鎖 N+M..N+2M-1、椅子 N+2M..N+3M-1、s、t
  Graph g;
  const int P = N, S = N + M, Ch = N + 2 * M;
  g.n = N + 3 * M + 2, g.s = N + 3 * M, g.t = N + 3 * M + 1;
  for (int i = 0; i < N; ++i) {
    int l = (int)rng.range(0, M - 1), r = (int)rng.range(0, M - 1);
    if (l > r) swap(l, r);
    if (l == r) { if (r + 1 < M) ++r; else --l; }
    g.e.push_back({g.s, i, 1, 0});
    if (l >= 0) g.e.push_back({i, P + l, N, 0});
    g.e.push_back({i, S + r, N, 0});
  }
  for (int k = 0; k < M; ++k) {
    if (k > 0) g.e.push_back({P + k, P + k - 1, N, 0});
    if (k + 1 < M) g.e.push_back({S + k, S + k + 1, N, 0});
    g.e.push_back({P + k, Ch + k, N, 0}), g.e.push_back({S + k, Ch + k, N, 0});
    g.e.push_back({Ch + k, g.t, 1, 0});
  }
  return g;
}

Graph segtree(Rng &rng, int N, int M) {
  // 頂点: 人 0..N-1、セグメント木の節 N + 1 .. N + 2 sz - 1 (節 x は N + x、葉 sz + k が椅子 k)、s、t
  Graph g;
  int sz = 1;
  while (sz < M) sz <<= 1;
  g.n = N + 2 * sz + 2, g.s = N + 2 * sz, g.t = N + 2 * sz + 1;
  for (int x = 1; x < sz; ++x) g.e.push_back({N + x, N + 2 * x, N, 0}), g.e.push_back({N + x, N + 2 * x + 1, N, 0});
  for (int k = 0; k < M; ++k) g.e.push_back({N + sz + k, g.t, 1, 0});
  const int maxlen = max(1, M / 8);
  for (int i = 0; i < N; ++i) {
    g.e.push_back({g.s, i, 1, 0});
    const int len = (int)rng.range(1, maxlen), l = (int)rng.range(0, M - len), r = l + len;  // [l, r)
    for (int a = l + sz, b = r + sz; a < b; a >>= 1, b >>= 1) {
      if (a & 1) g.e.push_back({i, N + a++, N, 0});
      if (b & 1) g.e.push_back({i, N + --b, N, 0});
    }
  }
  return g;
}

Graph closure(Rng &rng, int P, int Q, int D, int C) {
  Graph g;
  g.n = P + Q + 2, g.s = P + Q, g.t = P + Q + 1;
  vector<int> mach(Q);
  iota(mach.begin(), mach.end(), 0);
  for (int i = 0; i < P; ++i) {
    g.e.push_back({g.s, i, (int)rng.range(1, (i64)D * C), 0});
    for (int k = 0; k < D; ++k) swap(mach[k], mach[k + rng.below(Q - k)]);  // 先頭 D 個を乱択に選ぶ
    for (int k = 0; k < D; ++k) g.e.push_back({i, P + mach[k], (int)rng.range(1, C), 0});
  }
  for (int j = 0; j < Q; ++j) g.e.push_back({P + j, g.t, (int)rng.range(1, max<i64>(1, (i64)P * D * C / Q)), 0});
  return g;
}
}  // namespace gen

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
  if (fam == "cgrid") { auto a = args(3); g = gen::cgrid(rng, a[0], a[1], a[2]); }
  else if (fam == "cgrid_dir") { auto a = args(3); g = gen::cgrid_dir(rng, a[0], a[1], a[2]); }
  else if (fam == "presuf") { auto a = args(2); g = gen::presuf(rng, a[0], a[1]); }
  else if (fam == "segtree") { auto a = args(2); g = gen::segtree(rng, a[0], a[1]); }
  else if (fam == "closure") { auto a = args(4); g = gen::closure(rng, a[0], a[1], a[2], a[3]); }
  else {
    fprintf(stderr, "unknown family %s\n", fam.c_str());
    return 1;
  }

  auto t0 = chrono::steady_clock::now();
  Solver sol(g.n, g.s, g.t, g.e);
  sol.run();
  auto t1 = chrono::steady_clock::now();

  printf("%lld\n", (long long)sol.answer());
  report_metrics(
      (long long)chrono::duration_cast<chrono::nanoseconds>(t1 - t0).count());
}
