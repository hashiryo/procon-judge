// 自作の問題。最小費用の b-flow を、性質の違うグラフの族で測る。
//
// 提出は次を実装する (yosupo-min-cost-b-flow と同じ)。
//   struct Solver {
//     Solver(int n, const vector<i64> &b, const vector<array<i64, 5>> &edges);  // 辺は {s, t, 下限, 上限, 費用}
//     void run();                             // 最小費用の b-flow を求める
//     bool feasible() const;                  // b-flow があるか
//     __int128 cost() const;                  // 費用の和
//     const vector<i64> &potential() const;   // 頂点ごとのポテンシャル p
//     const vector<i64> &flow() const;        // 辺ごとの流量
//   };
//
// 入力は 1 行で、グラフの族の名前、seed、引数を並べる。グラフはハーネスが計測の前に作り、どの族も b-flow がある。
// 構築と run を測る。計測のあとに、流量が下限と上限の間にあること、各頂点の収支が b に等しいこと、相補性 (縮約費用
// c + p_s - p_t が正の辺は下限まで、負の辺は上限まで流れている) と、費用の和が cost() に等しいことを確かめる。
// どれかが崩れていれば "invalid" と理由を出す (期待出力と合わないので WA になる)。b と edges は計測区間のあとまで
// 生きているので、提出は参照を持ってよい。
//
//   netgen N M S T F C U  NETGEN 風。頂点 0 から S - 1 が湧き出し、N - T から N - 1 が吸い込みで、合わせて F を
//                         乱択に分ける。残りの頂点を乱択に S 本の鎖に分け、湧き出し i から鎖 i をたどる辺と、鎖の
//                         終わりから次の鎖の始めへの辺 (どれも容量 F、費用 [1, C]) で、全部を 1 つの輪にする。吸い込み
//                         には、乱択の鎖の頂点から容量 F の辺を張る。残りは乱択の辺 (容量 [1, U]、費用 [1, C]) で、
//                         辺の数を M にする
//   assign N D C          疎な割り当て。左 N 頂点 (湧き出し 1) と右 N 頂点 (吸い込み 1)。左 i から、乱択の置換の行き先と
//                         乱択の右 D 頂点へ、容量 1、費用 [0, C] の辺。辺の順は乱択に並べ替える
//   transport H W S C     格子の輸送問題。H × W のマスに [-S, S] の湧き出しを置き (和が 0 になるよう最後のマスで合わせる)、
//                         隣り合うマスの間に向きごとに容量無限 (湧き出しの絶対値の和)、費用 [1, C] の辺
//   stflow N M K F C U    s から t へ F 流す形。N 頂点に乱択の辺 M 本 (容量 [1, U]、費用 [1, C])、s から乱択の K 頂点へと、
//                         乱択の K 頂点から t へ、容量 U、費用 0 の辺。どの道より高い費用 C N の辺 s → t を容量 F で張る
//   mixed N M C U         負の費用と下限を混ぜた形。乱択の辺 (自己ループもある) ごとに流量 f を [0, U] から選び、下限を
//                         [0, f]、上限を [f, U] から、費用を [-C, C] から選ぶ。b は f の収支にする
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
  vector<i64> b;
  vector<array<i64, 5>> e;
};

// total を k 個の非負の整数に乱択に分ける。
vector<i64> split(Rng &rng, i64 total, int k) {
  vector<i64> cut(k - 1);
  for (auto &c : cut) c = rng.range(0, total);
  sort(cut.begin(), cut.end());
  vector<i64> r(k);
  i64 prev = 0;
  for (int i = 0; i < k - 1; ++i) r[i] = cut[i] - prev, prev = cut[i];
  r[k - 1] = total - prev;
  return r;
}

template <class T> void shuffle(Rng &rng, vector<T> &v) {
  for (size_t i = v.size(); i > 1; --i) swap(v[i - 1], v[rng.below(i)]);
}

Graph netgen(Rng &rng, int N, int M, int S, int T, i64 F, i64 C, i64 U) {
  Graph g;
  g.n = N, g.b.assign(N, 0);
  auto sup = split(rng, F, S), dem = split(rng, F, T);
  for (int i = 0; i < S; ++i) g.b[i] = sup[i];
  for (int i = 0; i < T; ++i) g.b[N - T + i] = -dem[i];
  vector<int> mid(N - S - T);
  iota(mid.begin(), mid.end(), S);
  shuffle(rng, mid);
  // 鎖 i は mid[st[i], st[i + 1])。湧き出し i から鎖 i をたどり、鎖の終わりから湧き出し i + 1 へつないで輪にする。
  vector<int> st(S + 1);
  for (int i = 0; i <= S; ++i) st[i] = (int)((i64)mid.size() * i / S);
  for (int i = 0; i < S; ++i) {
    int prev = i;
    for (int k = st[i]; k < st[i + 1]; ++k) g.e.push_back({prev, mid[k], 0, F, rng.range(1, C)}), prev = mid[k];
    g.e.push_back({prev, (i + 1) % S, 0, F, rng.range(1, C)});
  }
  for (int i = 0; i < T; ++i) {
    const int from = mid.empty() ? (int)rng.below(S) : mid[rng.below(mid.size())];
    g.e.push_back({from, N - T + i, 0, F, rng.range(1, C)});
  }
  while ((int)g.e.size() < M) {
    const int u = (int)rng.below(N), v = (int)rng.below(N);
    if (u != v) g.e.push_back({u, v, 0, rng.range(1, U), rng.range(1, C)});
  }
  return g;
}

Graph assign(Rng &rng, int N, int D, i64 C) {
  Graph g;
  g.n = 2 * N, g.b.assign(2 * N, 0);
  for (int i = 0; i < N; ++i) g.b[i] = 1, g.b[N + i] = -1;
  vector<int> perm(N);
  iota(perm.begin(), perm.end(), 0);
  shuffle(rng, perm);
  for (int i = 0; i < N; ++i) {
    g.e.push_back({i, N + perm[i], 0, 1, rng.range(0, C)});
    for (int k = 0; k < D; ++k) g.e.push_back({i, N + rng.below(N), 0, 1, rng.range(0, C)});
  }
  shuffle(rng, g.e);
  return g;
}

Graph transport(Rng &rng, int H, int W, i64 S, i64 C) {
  Graph g;
  g.n = H * W, g.b.assign(H * W, 0);
  i64 sum = 0, abs_sum = 0;
  for (int v = 0; v + 1 < H * W; ++v) g.b[v] = rng.range(-S, S), sum += g.b[v];
  g.b[H * W - 1] = -sum;
  for (i64 x : g.b) abs_sum += x < 0 ? -x : x;
  const i64 cap = max<i64>(abs_sum, 1);
  for (int i = 0; i < H; ++i)
    for (int j = 0; j < W; ++j) {
      const int v = i * W + j;
      if (j + 1 < W) g.e.push_back({v, v + 1, 0, cap, rng.range(1, C)}), g.e.push_back({v + 1, v, 0, cap, rng.range(1, C)});
      if (i + 1 < H) g.e.push_back({v, v + W, 0, cap, rng.range(1, C)}), g.e.push_back({v + W, v, 0, cap, rng.range(1, C)});
    }
  return g;
}

Graph stflow(Rng &rng, int N, int M, int K, i64 F, i64 C, i64 U) {
  Graph g;
  const int s = N, t = N + 1;
  g.n = N + 2, g.b.assign(N + 2, 0);
  g.b[s] = F, g.b[t] = -F;
  for (int i = 0; i < M; ++i) {
    const int u = (int)rng.below(N), v = (int)rng.below(N);
    g.e.push_back({u, v, 0, rng.range(1, U), rng.range(1, C)});
  }
  for (int i = 0; i < K; ++i) g.e.push_back({s, rng.below(N), 0, U, 0});
  for (int i = 0; i < K; ++i) g.e.push_back({rng.below(N), t, 0, U, 0});
  g.e.push_back({s, t, 0, F, C * N});
  return g;
}

Graph mixed(Rng &rng, int N, int M, i64 C, i64 U) {
  Graph g;
  g.n = N, g.b.assign(N, 0);
  for (int i = 0; i < M; ++i) {
    const int u = (int)rng.below(N), v = (int)rng.below(N);
    const i64 f = rng.range(0, U), lo = rng.range(0, f), up = rng.range(f, U);
    g.e.push_back({u, v, lo, up, rng.range(-C, C)});
    g.b[u] += f, g.b[v] -= f;
  }
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

// 提出の答えを確かめる。崩れていれば理由を返す。
static string verify(const gen::Graph &g, const Solver &sol) {
  const auto &f = sol.flow();
  const auto &p = sol.potential();
  const size_t m = g.e.size();
  if (f.size() != m) return "flow の長さが辺の数と違う";
  if (p.size() != (size_t)g.n) return "potential の長さが頂点の数と違う";
  vector<i64> bal(g.n, 0);
  __int128 cost = 0;
  for (size_t i = 0; i < m; ++i) {
    const auto &[s, t, lo, up, c] = g.e[i];
    if (f[i] < lo || f[i] > up) return "辺 " + to_string(i) + " の流量が範囲の外";
    bal[s] += f[i], bal[t] -= f[i];
    cost += (__int128)f[i] * c;
    const __int128 rc = (__int128)c + p[s] - p[t];
    if (rc > 0 && f[i] != lo) return "辺 " + to_string(i) + " の縮約費用が正なのに下限より多く流れている";
    if (rc < 0 && f[i] != up) return "辺 " + to_string(i) + " の縮約費用が負なのに上限まで流れていない";
  }
  for (int v = 0; v < g.n; ++v)
    if (bal[v] != g.b[v]) return "頂点 " + to_string(v) + " の収支が b と違う";
  if (cost != sol.cost()) return "cost() が流量から求めた費用の和と違う";
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
  if (fam == "netgen") { auto a = args(7); g = gen::netgen(rng, a[0], a[1], a[2], a[3], a[4], a[5], a[6]); }
  else if (fam == "assign") { auto a = args(3); g = gen::assign(rng, a[0], a[1], a[2]); }
  else if (fam == "transport") { auto a = args(4); g = gen::transport(rng, a[0], a[1], a[2], a[3]); }
  else if (fam == "stflow") { auto a = args(6); g = gen::stflow(rng, a[0], a[1], a[2], a[3], a[4], a[5]); }
  else if (fam == "mixed") { auto a = args(4); g = gen::mixed(rng, a[0], a[1], a[2], a[3]); }
  else {
    fprintf(stderr, "unknown family %s\n", fam.c_str());
    return 1;
  }

  auto t0 = chrono::steady_clock::now();
  Solver sol(g.n, g.b, g.e);
  sol.run();
  auto t1 = chrono::steady_clock::now();

  if (!sol.feasible()) printf("infeasible\n");
  else if (string why = verify(g, sol); !why.empty()) printf("invalid: %s\n", why.c_str());
  else printf("%s\n", i128_to_string(sol.cost()).c_str());

  report_metrics(
      (long long)chrono::duration_cast<chrono::nanoseconds>(t1 - t0).count());
}
