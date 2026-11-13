// 自作の問題。s から t へ流す量ごとの最小費用 (slope) を、性質の違うグラフの族で測る。NeoLibrary に置く、逐次最短路で
// slope を出す型 (ACL の mcf_graph と同じ役目) の中身を選ぶために作った。記録は algo-notes の notes/network-flow.md。
//
// 提出は次を実装する。
//   struct Solver {
//     explicit Solver(int n);                                 // 頂点 0 から n - 1
//     int add_vertex();                                       // 頂点を 1 つ足し、番号を返す
//     int add_edge(int from, int to, i64 cap, i64 cost);      // 辺を足し、番号 (0 から順) を返す。費用は負でもよい
//     vector<pair<i64, i64>> slope(int s, int t, i64 limit);  // 今の流れから続けて、s から t へ limit まで流す
//     i64 flow(int i) const;                                  // 辺 i の今の流量
//   };
//
// slope は、続けて流す量 x ごとの費用の増え方の最小 g(x) (下に凸の折れ線) の折れ点 (x, g(x)) を、(0, 0) から x の昇順に
// 返す。最後の点は流した量の全部で、流せるだけ流すか limit に達するまで流す。傾きの同じ点が続いてもよい (ハーネスが除いて
// から比べる)。slope を呼ぶ時点の残余グラフ (それまでに足した辺を含む) に負閉路が無いことは、ハーネスが保証する。辺と頂点は
// slope のあとにも足し、s と t は呼ぶたびに変わることがある。limit は流せるだけ流すとき i64 の最大になる。
//
// 入力は 1 行で、族の名前、seed、引数を並べる。グラフはハーネスが計測の前に作る (cooking だけは流しながら作る)。計測する
// のは、Solver の構築、add_vertex、add_edge、slope と、cooking で流しながら呼ぶ flow。計測のあとに、流量が 0 以上容量以下で
// あること、各頂点の収支 (slope ごとに s から出て t に入った量の和)、費用の和が slope の最後の点の費用の和に等しいことと、
// 折れ線が (0, 0) から始まって下に凸であることを確かめ、崩れていれば "invalid" と理由を出す。出力は、slope の回数、流した
// 量の和、費用の和、傾きの同じ点を除いた折れ点の数の和、折れ点の列のハッシュ。環境変数 PJ_DUMP があれば、傾きの同じ点を
// 除いた折れ点を stderr に出す。
//
//   random N M U C L SH     乱択の有向グラフ (洛谷 P3381 の形)。M 本の辺 (u != v、容量 [1, U]、費用 [0, C])。s = 0、
//                           t = N - 1。L まで流す (L = 0 なら流せるだけ)
//   assign L R D C G SH     二部グラフの割り当て。左 L 頂点のそれぞれから右の乱択の D 頂点へ容量 1 の辺。費用は [0, C] で、
//                           G = 1 なら符号を反転する (重みの最大。ABC 247 G の形)。s から左へ、右から t へ容量 1、費用 0
//   grid H W C SH           格子の完全マッチング (CF 316C2 の形)。市松の黒のマスから隣の白のマスへ容量 1、費用 [0, C]
//   dagpaths N K C SH       互いに素な K 本の広義単調増加の部分列の和の最大 (HDU 6611 の形)。a_i は [1, C]。頂点を入口と
//                           出口に分け、入口 → 出口 (容量 1、費用 -a_i)、出口 i → 入口 j (i < j かつ a_i <= a_j、容量 1、
//                           費用 0)、s → 入口、出口 → t (容量 1、費用 0)。K まで流す
//   intervals T N K C D SH  区間の K 重の詰め込みの重みの最大。0 から T の鎖の辺 i → i + 1 (容量 K、費用 0) と、区間
//                           [l, r) ごとの辺 l → r (容量 1、費用 -w、w は [1, C]、長さは [1, D])。s = 0、t = T。K まで流す
//   dense N U C             完全な有向グラフ (CF 362E の形)。容量 [1, U]、費用 [0, C]。s = 0、t = N - 1
//   gridflow H W U C        H × W の格子の流れ。隣のマスへ両向きに容量 [1, U]、費用 [1, C]。s から左の列へ、右の列から t へ
//                           容量 4 U、費用 0
//   expand N M K U C        網络扩容 (洛谷 P2604 の形) の 2 段。乱択の M 本の辺 (容量 [1, U]、費用 0) の網で頂点 0 から N - 1
//                           へ流せるだけ流したあと、各辺に並べて容量 K、費用 [1, C] の辺を足し、新しい頂点 s' から頂点 0 へ
//                           容量 K、費用 0 の辺を張って、s' から N - 1 へ K まで流す
//   cooking N M P C         美食节 (NOI2012、LOJ 2674 の形)。料理 N 種 (合わせて P 皿) と料理人 M 人。料理人 j が後ろから
//                           k 番目に作る料理 i の費用は k t_ij (t_ij は [1, C])。料理人ごとにまだ使っていない枠を 1 つだけ
//                           置き、1 皿ずつ流して、使った枠の次の枠を足す
//
// SH = 1 なら、頂点の番号と辺の順を乱択に並べ替える (番号の付け方の向きに頼る実装を見分けるため)。
#include "pj.hpp"
#include <algorithm>
#include <limits>
#include <numeric>
#include <string>
#include <utility>

#ifndef SUBMISSION_HPP
#define SUBMISSION_HPP "reference.hpp"
#endif
#include SUBMISSION_HPP

namespace gen {
constexpr i64 INF = numeric_limits<i64>::max();

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

template <class T> void shuffle(Rng &rng, vector<T> &v) {
  for (size_t i = v.size(); i > 1; --i) swap(v[i - 1], v[rng.below(i)]);
}

// 1 回の slope の前に足す頂点と辺と、流す s、t、上限。
struct Phase {
  int add_vertices = 0;
  vector<array<i64, 4>> edges;  // {from, to, 容量, 費用}
  int s = 0, t = 0;
  i64 limit = INF;
};

struct Instance {
  int n = 0;
  vector<Phase> phases;
};

// 1 段だけの網の頂点の番号と辺の順を並べ替える。
void relabel(Rng &rng, Instance &ins) {
  vector<int> perm(ins.n);
  iota(perm.begin(), perm.end(), 0);
  shuffle(rng, perm);
  for (Phase &ph : ins.phases) {
    for (auto &e : ph.edges) e[0] = perm[e[0]], e[1] = perm[e[1]];
    shuffle(rng, ph.edges);
    ph.s = perm[ph.s], ph.t = perm[ph.t];
  }
}

Instance random_graph(Rng &rng, int N, int M, i64 U, i64 C, i64 L) {
  Instance ins;
  ins.n = N;
  Phase ph;
  while ((int)ph.edges.size() < M) {
    const int u = (int)rng.below(N), v = (int)rng.below(N);
    if (u != v) ph.edges.push_back({u, v, rng.range(1, U), rng.range(0, C)});
  }
  ph.s = 0, ph.t = N - 1, ph.limit = L == 0 ? INF : L;
  ins.phases.push_back(ph);
  return ins;
}

Instance assign(Rng &rng, int L, int R, int D, i64 C, int G) {
  Instance ins;
  ins.n = L + R + 2;
  Phase ph;
  const int s = 0, t = L + R + 1;
  for (int i = 0; i < L; ++i) ph.edges.push_back({s, 1 + i, 1, 0});
  for (int j = 0; j < R; ++j) ph.edges.push_back({1 + L + j, t, 1, 0});
  for (int i = 0; i < L; ++i)
    for (int k = 0; k < D; ++k) {
      const i64 c = rng.range(0, C);
      ph.edges.push_back({1 + i, 1 + L + rng.below(R), 1, G ? -c : c});
    }
  ph.s = s, ph.t = t;
  ins.phases.push_back(ph);
  return ins;
}

Instance grid(Rng &rng, int H, int W, i64 C) {
  Instance ins;
  ins.n = H * W + 2;
  Phase ph;
  const int s = 0, t = H * W + 1;
  auto id = [&](int i, int j) { return 1 + i * W + j; };
  for (int i = 0; i < H; ++i)
    for (int j = 0; j < W; ++j) {
      if ((i + j) % 2) {
        ph.edges.push_back({id(i, j), t, 1, 0});
        continue;
      }
      ph.edges.push_back({s, id(i, j), 1, 0});
      const int di[] = {0, 1, 0, -1}, dj[] = {1, 0, -1, 0};
      for (int d = 0; d < 4; ++d) {
        const int a = i + di[d], b = j + dj[d];
        if (0 <= a && a < H && 0 <= b && b < W) ph.edges.push_back({id(i, j), id(a, b), 1, rng.range(0, C)});
      }
    }
  ph.s = s, ph.t = t;
  ins.phases.push_back(ph);
  return ins;
}

Instance dagpaths(Rng &rng, int N, int K, i64 C) {
  Instance ins;
  ins.n = 2 * N + 2;
  Phase ph;
  vector<i64> a(N);
  for (auto &x : a) x = rng.range(1, C);
  for (int i = 0; i < N; ++i) {
    ph.edges.push_back({0, 2 + 2 * i, 1, 0});
    ph.edges.push_back({2 + 2 * i, 3 + 2 * i, 1, -a[i]});
    ph.edges.push_back({3 + 2 * i, 1, 1, 0});
    for (int j = i + 1; j < N; ++j)
      if (a[i] <= a[j]) ph.edges.push_back({3 + 2 * i, 2 + 2 * j, 1, 0});
  }
  ph.s = 0, ph.t = 1, ph.limit = K;
  ins.phases.push_back(ph);
  return ins;
}

Instance intervals(Rng &rng, int T, int N, int K, i64 C, int D) {
  Instance ins;
  ins.n = T + 1;
  Phase ph;
  for (int i = 0; i < T; ++i) ph.edges.push_back({i, i + 1, K, 0});
  for (int k = 0; k < N; ++k) {
    const int l = (int)rng.below(T), r = (int)min<i64>(T, l + rng.range(1, D));
    ph.edges.push_back({l, r, 1, -rng.range(1, C)});
  }
  ph.s = 0, ph.t = T, ph.limit = K;
  ins.phases.push_back(ph);
  return ins;
}

Instance dense(Rng &rng, int N, i64 U, i64 C) {
  Instance ins;
  ins.n = N;
  Phase ph;
  for (int u = 0; u < N; ++u)
    for (int v = 0; v < N; ++v)
      if (u != v) ph.edges.push_back({u, v, rng.range(1, U), rng.range(0, C)});
  ph.s = 0, ph.t = N - 1;
  ins.phases.push_back(ph);
  return ins;
}

Instance gridflow(Rng &rng, int H, int W, i64 U, i64 C) {
  Instance ins;
  ins.n = H * W + 2;
  Phase ph;
  const int s = H * W, t = H * W + 1;
  for (int i = 0; i < H; ++i)
    for (int j = 0; j < W; ++j) {
      const int v = i * W + j;
      if (j + 1 < W) {
        ph.edges.push_back({v, v + 1, rng.range(1, U), rng.range(1, C)});
        ph.edges.push_back({v + 1, v, rng.range(1, U), rng.range(1, C)});
      }
      if (i + 1 < H) {
        ph.edges.push_back({v, v + W, rng.range(1, U), rng.range(1, C)});
        ph.edges.push_back({v + W, v, rng.range(1, U), rng.range(1, C)});
      }
    }
  for (int i = 0; i < H; ++i) ph.edges.push_back({s, i * W, 4 * U, 0}), ph.edges.push_back({i * W + W - 1, t, 4 * U, 0});
  ph.s = s, ph.t = t;
  ins.phases.push_back(ph);
  return ins;
}

Instance expand(Rng &rng, int N, int M, i64 K, i64 U, i64 C) {
  Instance ins;
  ins.n = N;
  Phase p1, p2;
  while ((int)p1.edges.size() < M) {
    const int u = (int)rng.below(N), v = (int)rng.below(N);
    if (u != v) p1.edges.push_back({u, v, rng.range(1, U), 0});
  }
  p1.s = 0, p1.t = N - 1;
  p2.add_vertices = 1;
  for (const auto &e : p1.edges) p2.edges.push_back({e[0], e[1], K, rng.range(1, C)});
  p2.edges.push_back({N, 0, K, 0});
  p2.s = N, p2.t = N - 1, p2.limit = K;
  ins.phases.push_back(p1), ins.phases.push_back(p2);
  return ins;
}
}  // namespace gen

namespace check {
struct Call {
  int s, t;
  vector<pair<i64, i64>> pts;  // 傾きの同じ点を除いたもの
};

// 折れ線を確かめ、傾きの同じ点と長さ 0 の区間を除く。崩れていれば理由を返す。
string canonical(const vector<pair<i64, i64>> &raw, i64 limit, vector<pair<i64, i64>> &out) {
  if (raw.empty() || raw[0] != pair<i64, i64>{0, 0}) return "slope が (0, 0) から始まっていない";
  out.clear();
  out.push_back(raw[0]);
  for (size_t k = 1; k < raw.size(); ++k) {
    const auto [x, y] = raw[k];
    const auto [px, py] = out.back();
    if (x < px) return "slope の流量が減っている";
    if (x == px) {
      if (y != py) return "slope の同じ流量に違う費用がある";
      continue;
    }
    if (out.size() >= 2) {
      const auto [qx, qy] = out[out.size() - 2];
      // (py - qy) / (px - qx) と (y - py) / (x - px) を比べる
      const __int128 l = (__int128)(py - qy) * (x - px), r = (__int128)(y - py) * (px - qx);
      if (r < l) return "slope が下に凸でない";
      if (r == l) out.pop_back();
    }
    out.push_back({x, y});
  }
  if (out.back().first > limit) return "slope が上限より多く流している";
  return "";
}
}  // namespace check

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
  gen::Instance ins;
  bool cooking = false;
  vector<i64> ck;
  int sh = 0;
  if (fam == "random") { auto a = args(6); ins = gen::random_graph(rng, a[0], a[1], a[2], a[3], a[4]), sh = a[5]; }
  else if (fam == "assign") { auto a = args(6); ins = gen::assign(rng, a[0], a[1], a[2], a[3], a[4]), sh = a[5]; }
  else if (fam == "grid") { auto a = args(4); ins = gen::grid(rng, a[0], a[1], a[2]), sh = a[3]; }
  else if (fam == "dagpaths") { auto a = args(4); ins = gen::dagpaths(rng, a[0], a[1], a[2]), sh = a[3]; }
  else if (fam == "intervals") { auto a = args(6); ins = gen::intervals(rng, a[0], a[1], a[2], a[3], a[4]), sh = a[5]; }
  else if (fam == "dense") { auto a = args(3); ins = gen::dense(rng, a[0], a[1], a[2]); }
  else if (fam == "gridflow") { auto a = args(4); ins = gen::gridflow(rng, a[0], a[1], a[2], a[3]); }
  else if (fam == "expand") { auto a = args(5); ins = gen::expand(rng, a[0], a[1], a[2], a[3], a[4]); }
  else if (fam == "cooking") { ck = args(4), cooking = true; }
  else {
    fprintf(stderr, "unknown family %s\n", fam.c_str());
    return 1;
  }
  if (sh) gen::relabel(rng, ins);

  // cooking の料理ごとの皿の数と、料理人ごとの時間。料理 i は頂点 2 + i、s = 0、t = 1。
  int cN = 0, cM = 0, cP = 0;
  vector<i64> cp;
  vector<vector<i64>> ct;
  if (cooking) {
    cN = (int)ck[0], cM = (int)ck[1], cP = (int)ck[2];
    cp.assign(cN, 1);
    for (int k = cN; k < cP; ++k) ++cp[rng.below(cN)];
    ct.assign(cM, vector<i64>(cN));
    for (auto &row : ct)
      for (auto &x : row) x = rng.range(1, ck[3]);
    ins.n = 2 + cN;
  }

  // 辺の一覧 (ハーネスが確かめに使う) と、slope ごとの結果。
  vector<array<i64, 4>> edges;
  vector<check::Call> calls;
  vector<pair<vector<pair<i64, i64>>, i64>> raws;  // 生の折れ線と上限
  int n = ins.n;

  auto t0 = chrono::steady_clock::now();
  Solver sol(ins.n);
  auto add_edge = [&](i64 u, i64 v, i64 cap, i64 cost) {
    const int id = sol.add_edge((int)u, (int)v, cap, cost);
    if (id != (int)edges.size()) {
      fprintf(stderr, "add_edge が番号 %d を返した (%zu のはず)\n", id, edges.size());
      exit(1);
    }
    edges.push_back({u, v, cap, cost});
  };
  auto add_vertex = [&] {
    const int id = sol.add_vertex();
    if (id != n) {
      fprintf(stderr, "add_vertex が番号 %d を返した (%d のはず)\n", id, n);
      exit(1);
    }
    return n++;
  };
  if (!cooking) {
    for (const gen::Phase &ph : ins.phases) {
      for (int k = 0; k < ph.add_vertices; ++k) add_vertex();
      for (const auto &e : ph.edges) add_edge(e[0], e[1], e[2], e[3]);
      raws.push_back({sol.slope(ph.s, ph.t, ph.limit), ph.limit});
      calls.push_back({ph.s, ph.t, {}});
    }
  } else {
    for (int i = 0; i < cN; ++i) add_edge(0, 2 + i, cp[i], 0);
    // 料理人ごとの、まだ使っていない枠の t への辺の番号と、その枠が後ろから何番目か。
    vector<int> front(cM), kth(cM, 1);
    auto add_slot = [&](int j) {
      const int v = add_vertex();
      for (int i = 0; i < cN; ++i) add_edge(2 + i, v, 1, kth[j] * ct[j][i]);
      front[j] = (int)edges.size();
      add_edge(v, 1, 1, 0);
    };
    for (int j = 0; j < cM; ++j) add_slot(j);
    for (int k = 0; k < cP; ++k) {
      raws.push_back({sol.slope(0, 1, 1), 1});
      calls.push_back({0, 1, {}});
      for (int j = 0; j < cM; ++j)
        if (sol.flow(front[j]) != 0) ++kth[j], add_slot(j);
    }
  }
  auto t1 = chrono::steady_clock::now();

  // 確かめる。
  string why;
  vector<i64> bal(n, 0), expect(n, 0);
  i64 total_flow = 0, total_cost = 0, points = 0;
  u64 hash = 0;
  for (size_t k = 0; k < calls.size() && why.empty(); ++k) {
    why = check::canonical(raws[k].first, raws[k].second, calls[k].pts);
    if (!why.empty()) break;
    const auto [f, c] = calls[k].pts.back();
    expect[calls[k].s] += f, expect[calls[k].t] -= f;
    total_flow += f, total_cost += c, points += (i64)calls[k].pts.size();
    hash = hash * 1000003 + 0x5bd1e995;
    for (const auto &[x, y] : calls[k].pts) hash = (hash * 1000003 + (u64)x) * 1000003 + (u64)y;
  }
  if (why.empty()) {
    __int128 cost = 0;
    for (size_t i = 0; i < edges.size(); ++i) {
      const i64 f = sol.flow((int)i);
      const auto &[u, v, cap, c] = edges[i];
      if (f < 0 || f > cap) {
        why = "辺 " + to_string(i) + " の流量が範囲の外";
        break;
      }
      bal[u] += f, bal[v] -= f, cost += (__int128)f * c;
    }
    if (why.empty()) {
      for (int v = 0; v < n && why.empty(); ++v)
        if (bal[v] != expect[v]) why = "頂点 " + to_string(v) + " の収支が slope の流量と合わない";
      if (why.empty() && cost != total_cost) why = "辺の流量の費用の和が slope の費用の和と違う";
    }
  }
  if (!why.empty()) printf("invalid: %s\n", why.c_str());
  else printf("%zu %lld %lld %lld %llu\n", calls.size(), total_flow, total_cost, points, (unsigned long long)hash);
  if (getenv("PJ_DUMP"))
    for (size_t k = 0; k < calls.size(); ++k) {
      fprintf(stderr, "call %zu (s %d, t %d):", k, calls[k].s, calls[k].t);
      for (const auto &[x, y] : calls[k].pts) fprintf(stderr, " (%lld, %lld)", x, y);
      fprintf(stderr, "\n");
    }

  report_metrics((long long)chrono::duration_cast<chrono::nanoseconds>(t1 - t0).count());
}
