// 自作の問題。最小費用の b-flow を、鎖、木、環、直線の形の網で測る。どれも、想定解は slope trick や貪欲や模擬費用流
// だが、網を作って最小費用流で解くこともできる問題の形にしてある (algo-notes の network-flow-brute-force.md)。
//
// 提出は self-flow-mincostflow と同じものを実装する。
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
// 費用流双対の形 (iso_chain と iso_tree) は、凸な関数の和をポテンシャルについて最小にする問題を b-flow に直したもの。
// 辺 u → v (容量 w、費用 c) は w max(0, p_v - p_u - c) の項に、容量無限の辺は p_v - p_u <= c の制約に当たり、
// 最小のポテンシャルの問題の値は b-flow の最小費用の符号を反転したものになる。
//
//   iso_chain N A D      列 a の L1 の保序回帰 (x_0 <= x_1 <= ... で sum |x_i - a_i| を最小にする。CF 713C、POJ 3666 の形)。
//                        a_i は i A / N に [-D, D] の乱択を足して [0, A] に収めたもの。頂点 0 から N - 1 が x_i、頂点 N が
//                        根 r (x_i = p_i - p_r)。辺は i + 1 → i (容量 N、費用 0。x_i <= x_{i+1})、r → i (容量 1、費用 a_i)、
//                        i → r (容量 1、費用 -a_i)。b は 0
//   iso_tree N A D K     木の上の L1 の保序回帰 (親 <= 子)。頂点 i (1 <= i < N) の親は [max(0, i - K), i - 1] の乱択で、
//                        K が小さいほど深い木になる。a_i は深さ d について d A / 深さの最大 に [-D, D] を足したもの。頂点の
//                        番号は根の 0 を除いて乱択に並べ替える。辺は子 → 親 (容量 N、費用 0) と、iso_chain と同じ根との 2 本
//   ring N A             輪の上の輸送 (糖果传递、UVa 11300 の形)。N 人が a_i (乱択、[0, A]) 個ずつ持ち、隣へ 1 個動かすと
//                        費用 1 で、全員を平均にする。b_i = a_i - 平均 (和が N で割れるように最後の人で合わせる)。辺は
//                        i → i + 1 と i + 1 → i (mod N、容量は湧き出しの和、費用 1)
//   buysell N A D        株の売買 (CF 865D、ABC 250 G の形)。値段は A / 2 から始めて毎日 [-D, D] 動かし、[1, A] に収める。
//                        日 i から s → i (容量 1、費用 p_i)、i → t (容量 1、費用 -p_i)、i → i + 1 (容量 N、費用 0)、
//                        t → s (容量 N、費用 0)。b は 0
//   deadline N M B       締切付きの仕事 (ABC 137 D の形)。仕事 i は M - A_i 日目まで (A_i は [1, M]) にやれば報酬 B_i
//                        ([1, B])。s → 仕事 (容量 1、費用 -B_i)、仕事 → 日 M - A_i (容量 1)、日 d → 日 d - 1 (容量 N)、
//                        日 → t (容量 1)、t → s (容量 N)。b は 0
//   fewsink N K A        吸い込みの少ない輸送 (AGC 018 C の形)。N 人が K 種の役のどれか 1 つに就き、人 i が役 k に就くと
//                        価値 a_ik ([0, A])。役 k の定員は N を K に分けたもの。s → 人 (容量 1)、人 → 役 (容量 1、費用 -a_ik)、
//                        役 → t (容量 定員)。b_s = N、b_t = -N
//   snow N M X W C       直線の上の輸送 (UOJ 455 雪灾与外卖の形)。N 匹の鼠と M 個の穴を [0, X] に置き、穴 j の容量は [1, C]
//                        (和が N 以上になるよう足す)、穴に入る費用は [0, W]。座標の順に並べた隣どうしを、両向きに容量 N、
//                        費用は座標の差の辺でつなぐ。鼠は b = 1、穴 → t (容量 c_j、費用 w_j)、b_t = -N
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

template <class T> void shuffle(Rng &rng, vector<T> &v) {
  for (size_t i = v.size(); i > 1; --i) swap(v[i - 1], v[rng.below(i)]);
}

i64 clamp_range(i64 x, i64 lo, i64 hi) { return x < lo ? lo : x > hi ? hi : x; }

// 頂点 0..N-1 の値 a と、根 N との 2 本の辺 (|x_i - a_i| の項)。
void add_root_terms(Graph &g, const vector<i64> &a) {
  const int N = (int)a.size(), r = N;
  for (int i = 0; i < N; ++i) g.e.push_back({r, i, 0, 1, a[i]}), g.e.push_back({i, r, 0, 1, -a[i]});
}

Graph iso_chain(Rng &rng, int N, i64 A, i64 D) {
  Graph g;
  g.n = N + 1, g.b.assign(N + 1, 0);
  vector<i64> a(N);
  for (int i = 0; i < N; ++i) a[i] = clamp_range((i64)((__int128)A * i / N) + rng.range(-D, D), 0, A);
  for (int i = 0; i + 1 < N; ++i) g.e.push_back({i + 1, i, 0, N, 0});
  add_root_terms(g, a);
  return g;
}

Graph iso_tree(Rng &rng, int N, i64 A, i64 D, int K) {
  Graph g;
  g.n = N + 1, g.b.assign(N + 1, 0);
  vector<int> par(N, -1), dep(N, 0);
  for (int i = 1; i < N; ++i) par[i] = (int)rng.range(max(0, i - K), i - 1), dep[i] = dep[par[i]] + 1;
  const int maxd = max(1, *max_element(dep.begin(), dep.end()));
  vector<int> id(N);
  iota(id.begin(), id.end(), 0);
  shuffle(rng, id), swap(id[0], *find(id.begin(), id.end(), 0));  // 根は頂点 0 のまま
  vector<i64> a(N);
  for (int i = 0; i < N; ++i) a[id[i]] = clamp_range((i64)((__int128)A * dep[i] / maxd) + rng.range(-D, D), 0, A);
  for (int i = 1; i < N; ++i) g.e.push_back({id[i], id[par[i]], 0, N, 0});
  add_root_terms(g, a);
  return g;
}

Graph ring(Rng &rng, int N, i64 A) {
  Graph g;
  g.n = N, g.b.assign(N, 0);
  vector<i64> a(N);
  i64 sum = 0;
  for (auto &x : a) x = rng.range(0, A), sum += x;
  a[N - 1] += (N - sum % N) % N, sum += (N - (sum % N)) % N;  // 和を N で割れるようにする
  const i64 avg = sum / N;
  i64 sup = 0;
  for (int i = 0; i < N; ++i) g.b[i] = a[i] - avg, sup += max<i64>(g.b[i], 0);
  const i64 cap = max<i64>(sup, 1);
  for (int i = 0; i < N; ++i) {
    const int j = (i + 1) % N;
    g.e.push_back({i, j, 0, cap, 1}), g.e.push_back({j, i, 0, cap, 1});
  }
  return g;
}

Graph buysell(Rng &rng, int N, i64 A, i64 D) {
  Graph g;
  const int s = N, t = N + 1;
  g.n = N + 2, g.b.assign(N + 2, 0);
  i64 p = A / 2;
  for (int i = 0; i < N; ++i) {
    p = clamp_range(p + rng.range(-D, D), 1, A);
    g.e.push_back({s, i, 0, 1, p}), g.e.push_back({i, t, 0, 1, -p});
    if (i + 1 < N) g.e.push_back({i, i + 1, 0, N, 0});
  }
  g.e.push_back({t, s, 0, N, 0});
  return g;
}

Graph deadline(Rng &rng, int N, int M, i64 B) {
  Graph g;
  const int s = N + M, t = N + M + 1;
  g.n = N + M + 2, g.b.assign(N + M + 2, 0);
  for (int i = 0; i < N; ++i) {
    const int A = (int)rng.range(1, M);
    g.e.push_back({s, i, 0, 1, -rng.range(1, B)}), g.e.push_back({i, N + M - A, 0, 1, 0});
  }
  for (int d = 0; d < M; ++d) {
    if (d > 0) g.e.push_back({N + d, N + d - 1, 0, N, 0});
    g.e.push_back({N + d, t, 0, 1, 0});
  }
  g.e.push_back({t, s, 0, N, 0});
  return g;
}

Graph fewsink(Rng &rng, int N, int K, i64 A) {
  Graph g;
  const int s = N + K, t = N + K + 1;
  g.n = N + K + 2, g.b.assign(N + K + 2, 0);
  g.b[s] = N, g.b[t] = -N;
  for (int i = 0; i < N; ++i) {
    g.e.push_back({s, i, 0, 1, 0});
    for (int k = 0; k < K; ++k) g.e.push_back({i, N + k, 0, 1, -rng.range(0, A)});
  }
  for (int k = 0; k < K; ++k) g.e.push_back({N + k, t, 0, (i64)N * (k + 1) / K - (i64)N * k / K, 0});
  return g;
}

Graph snow(Rng &rng, int N, int M, i64 X, i64 W, i64 C) {
  Graph g;
  const int t = N + M;
  g.n = N + M + 1, g.b.assign(N + M + 1, 0);
  vector<pair<i64, int>> pt(N + M);  // (座標, 頂点)。0..N-1 が鼠、N..N+M-1 が穴
  for (int i = 0; i < N + M; ++i) pt[i] = {rng.range(0, X), i};
  vector<i64> c(M);
  i64 total = 0;
  for (auto &x : c) x = rng.range(1, C), total += x;
  for (int j = 0; total < N; j = (j + 1) % M) ++c[j], ++total;
  for (int i = 0; i < N; ++i) g.b[i] = 1;
  g.b[t] = -N;
  for (int j = 0; j < M; ++j) g.e.push_back({N + j, t, 0, c[j], rng.range(0, W)});
  sort(pt.begin(), pt.end());
  for (int k = 0; k + 1 < N + M; ++k) {
    const auto [x0, u] = pt[k];
    const auto [x1, v] = pt[k + 1];
    g.e.push_back({u, v, 0, N, x1 - x0}), g.e.push_back({v, u, 0, N, x1 - x0});
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
  if (fam == "iso_chain") { auto a = args(3); g = gen::iso_chain(rng, a[0], a[1], a[2]); }
  else if (fam == "iso_tree") { auto a = args(4); g = gen::iso_tree(rng, a[0], a[1], a[2], a[3]); }
  else if (fam == "ring") { auto a = args(2); g = gen::ring(rng, a[0], a[1]); }
  else if (fam == "buysell") { auto a = args(3); g = gen::buysell(rng, a[0], a[1], a[2]); }
  else if (fam == "deadline") { auto a = args(3); g = gen::deadline(rng, a[0], a[1], a[2]); }
  else if (fam == "fewsink") { auto a = args(3); g = gen::fewsink(rng, a[0], a[1], a[2]); }
  else if (fam == "snow") { auto a = args(5); g = gen::snow(rng, a[0], a[1], a[2], a[3], a[4]); }
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
