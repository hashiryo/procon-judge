#pragma once
// 重み付きのマトロイドの交差の書き比べ。重みの和が最大の共通独立集合を、要素数 k = 0, 1, ... の順に 1 つずつ増やして
// 求める。増加路は、交換グラフで要素に長さ (I に無い x は -w(x)、I にある y は w(y)) を付けた最短路のうち、辺の数が
// 最も少ないもの。マトロイドの型は matroids.hpp と同じ 4 つの操作を持つ。
//   wmi_dijkstra<false>  ポテンシャル付きの Dijkstra。辺は 2 引数の交換の判定 (can_exchange) で、取り出した要素から
//                        反対側のまだ確定していない要素を全部調べて見つける
//   wmi_dijkstra<true>   ポテンシャル付きの Dijkstra。辺は閉路の列挙 (circuit) で毎回作った交換グラフから引く
//   wmi_spfa             閉路の列挙で作った交換グラフの上で、ポテンシャルなしに Bellman-Ford (待ち行列) で最短路を求める
// ポテンシャルは Frank の重みの分け方 w = w1 + w2 を表す。I にある y は pot(y) = w1(y)、I に無い x は pot(x) = -w2(x)。
// I が M1 で w1 の、M2 で w2 の最大のとき、交換グラフの辺の縮めた長さ (M1 の辺 y → x は w1(y) - w1(x)、M2 の辺 x → y は
// w2(y) - w2(x)) は 0 以上になる。
#include <algorithm>
#include <array>
#include <limits>
#include <queue>
#include <tuple>
#include <vector>

namespace wmi_algo {
using i64 = long long;

struct Result {
  std::vector<i64> best;  // best[k] は要素数 k の重みの和の最大
  std::vector<int> set;   // 要素数が最大で重みの和が best.back() の集合
};

// 縮めた長さと辺の数の組で比べる Dijkstra。始点は M1 に足せる要素、終点は M2 に足せる要素の先の仮の頂点 n。
template <bool CIRCUIT, class M1, class M2> Result wmi_dijkstra(int n, M1 &a, M2 &b, const std::vector<i64> &w) {
  const i64 INF = std::numeric_limits<i64>::max() / 4;
  std::vector<char> in(n), src(n), snk(n), done(n);
  std::vector<int> I, hop(n), par(n), outs, start(n + 1), to, path;
  std::vector<i64> pot(n, 0), d(n);
  std::vector<std::array<int, 2>> es;
  using T = std::tuple<i64, int, int>;
  std::priority_queue<T, std::vector<T>, std::greater<T>> pq;
  Result res;
  res.best.push_back(0);
  a.build(I), b.build(I);
  while (true) {
    i64 W1 = -INF, W2 = -INF;
    outs.clear();
    for (int x = 0; x < n; ++x) {
      src[x] = snk[x] = 0;
      if (in[x]) continue;
      if (a.can_add(x)) src[x] = 1, W1 = std::max(W1, w[x] + pot[x]);
      else outs.push_back(x);
      if (b.can_add(x)) snk[x] = 1, W2 = std::max(W2, -pot[x]);
    }
    if (W1 == -INF || W2 == -INF) break;
    if constexpr (CIRCUIT) {
      // y ∈ C1(x) なら y → x、y ∈ C2(x) なら x → y。始点に入る辺と終点から出る辺は最短路に要らないので作らない
      es.clear();
      for (int x : outs) a.circuit(x, [&](int y) { es.push_back({y, x}); });
      for (int x = 0; x < n; ++x)
        if (!in[x] && !snk[x]) b.circuit(x, [&](int y) { es.push_back({x, y}); });
      std::fill(start.begin(), start.end(), 0);
      for (auto &[u, v] : es) ++start[u + 1];
      for (int u = 0; u < n; ++u) start[u + 1] += start[u];
      to.resize(es.size());
      for (auto &[u, v] : es) to[start[u]++] = v;
      for (int u = n; u > 0; --u) start[u] = start[u - 1];
      start[0] = 0;
    }
    std::fill(d.begin(), d.end(), INF);
    std::fill(done.begin(), done.end(), 0);
    while (!pq.empty()) pq.pop();
    for (int x = 0; x < n; ++x)
      if (src[x]) d[x] = W1 - (w[x] + pot[x]), hop[x] = 0, par[x] = -1, pq.push({d[x], 0, x});
    i64 DT = INF;
    int hT = 0, tb = -1;
    while (!pq.empty()) {
      const auto [dv, hv, v] = pq.top();
      pq.pop();
      if (v == n) break;
      if (done[v] || dv != d[v] || hv != hop[v]) continue;
      done[v] = 1;
      auto relax = [&](int u, i64 c) {
        const i64 nd = dv + c;
        const int nh = hv + 1;
        if (nd < d[u] || (nd == d[u] && nh < hop[u])) d[u] = nd, hop[u] = nh, par[u] = v, pq.push({nd, nh, u});
      };
      if (!in[v]) {
        if (snk[v]) {
          const i64 nd = dv + W2 + pot[v];
          if (nd < DT || (nd == DT && hv + 1 < hT)) DT = nd, hT = hv + 1, tb = v, pq.push({DT, hT, n});
          continue;
        }
        // M2 の辺 v → y (y ∈ C2(v))。縮めた長さは w2(y) - w2(v) = (w[y] - pot[y]) + pot[v]
        if constexpr (CIRCUIT) {
          for (int k = start[v]; k < start[v + 1]; ++k)
            if (const int y = to[k]; !done[y]) relax(y, w[y] - pot[y] + pot[v]);
        } else {
          for (int y : I)
            if (!done[y] && b.can_exchange(y, v)) relax(y, w[y] - pot[y] + pot[v]);
        }
      } else {
        // M1 の辺 v → x (v ∈ C1(x))。縮めた長さは w1(v) - w1(x) = pot[v] - (w[x] + pot[x])
        if constexpr (CIRCUIT) {
          for (int k = start[v]; k < start[v + 1]; ++k)
            if (const int x = to[k]; !done[x]) relax(x, pot[v] - w[x] - pot[x]);
        } else {
          for (size_t k = 0; k < outs.size();) {
            const int x = outs[k];
            if (done[x]) {
              outs[k] = outs.back(), outs.pop_back();
              continue;
            }
            if (a.can_exchange(v, x)) relax(x, pot[v] - w[x] - pot[x]);
            ++k;
          }
        }
      }
    }
    if (tb < 0) break;
    // ポテンシャルを min(d, DT) だけ上げ (Frank の重みの分け方の更新)、増加路の上の要素は I の出入りに合わせて表し直す
    for (int v = 0; v < n; ++v) pot[v] += done[v] && d[v] < DT ? d[v] : DT;
    path.clear();
    for (int v = tb; v >= 0; v = par[v]) path.push_back(v);
    i64 gain = 0;
    for (int v : path) {
      if (in[v]) pot[v] -= w[v], gain -= w[v];
      else pot[v] += w[v], gain += w[v];
      in[v] ^= 1;
    }
    I.erase(std::remove_if(I.begin(), I.end(), [&](int y) { return !in[y]; }), I.end());
    for (int v : path)
      if (in[v]) I.push_back(v);
    a.build(I), b.build(I);
    res.best.push_back(res.best.back() + gain);
  }
  res.set = I;
  return res;
}

// 閉路の列挙で交換グラフを作り、要素の長さ (I に無い x は -w(x)、I にある y は w(y)) の和と辺の数の組で最小の路を
// Bellman-Ford (待ち行列) で求める。ポテンシャルは持たない。
template <class M1, class M2> Result wmi_spfa(int n, M1 &a, M2 &b, const std::vector<i64> &w) {
  const i64 INF = std::numeric_limits<i64>::max() / 4;
  std::vector<char> in(n), src(n), snk(n), inq(n);
  std::vector<int> I, hop(n), par(n), start(n + 1), to, path;
  std::vector<i64> d(n);
  std::vector<std::array<int, 2>> es;
  std::vector<int> q;
  Result res;
  res.best.push_back(0);
  a.build(I), b.build(I);
  while (true) {
    es.clear();
    for (int x = 0; x < n; ++x) {
      src[x] = snk[x] = 0;
      if (in[x]) continue;
      if (a.can_add(x)) src[x] = 1;
      else a.circuit(x, [&](int y) { es.push_back({y, x}); });
      if (b.can_add(x)) snk[x] = 1;
      else b.circuit(x, [&](int y) { es.push_back({x, y}); });
    }
    std::fill(start.begin(), start.end(), 0);
    for (auto &[u, v] : es) ++start[u + 1];
    for (int u = 0; u < n; ++u) start[u + 1] += start[u];
    to.resize(es.size());
    for (auto &[u, v] : es) to[start[u]++] = v;
    for (int u = n; u > 0; --u) start[u] = start[u - 1];
    start[0] = 0;
    std::fill(d.begin(), d.end(), INF);
    // 待ち行列は長さ n + 1 の輪 (同じ要素は同時に 1 つしか入らない)
    q.assign(n + 1, 0);
    int qh = 0, qt = 0;
    for (int x = 0; x < n; ++x)
      if (src[x]) d[x] = -w[x], hop[x] = 0, par[x] = -1, inq[x] = 1, q[qt++] = x;
    while (qh != qt) {
      const int u = q[qh];
      qh = qh == n ? 0 : qh + 1;
      inq[u] = 0;
      for (int k = start[u]; k < start[u + 1]; ++k) {
        const int v = to[k];
        if (src[v]) continue;
        const i64 nd = d[u] + (in[v] ? w[v] : -w[v]);
        const int nh = hop[u] + 1;
        if (nd < d[v] || (nd == d[v] && nh < hop[v])) {
          d[v] = nd, hop[v] = nh, par[v] = u;
          if (!inq[v]) inq[v] = 1, q[qt] = v, qt = qt == n ? 0 : qt + 1;
        }
      }
    }
    int tb = -1;
    for (int x = 0; x < n; ++x)
      if (snk[x] && d[x] < INF && (tb < 0 || d[x] < d[tb] || (d[x] == d[tb] && hop[x] < hop[tb]))) tb = x;
    if (tb < 0) break;
    path.clear();
    for (int v = tb; v >= 0; v = par[v]) path.push_back(v);
    for (int v : path) in[v] ^= 1;
    I.erase(std::remove_if(I.begin(), I.end(), [&](int y) { return !in[y]; }), I.end());
    for (int v : path)
      if (in[v]) I.push_back(v);
    a.build(I), b.build(I);
    res.best.push_back(res.best.back() - d[tb]);
  }
  res.set = I;
  return res;
}
}  // namespace wmi_algo
