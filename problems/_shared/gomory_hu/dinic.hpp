#pragma once
// Gusfield の方法の最大流の部分を、網を 1 回だけ組む Dinic で書いたもの (gusfield.hpp の Engine)。
// 無向辺 {u, v, c} は、u から v と v から u の容量 c の弧の組にし、互いを逆向きの弧とする。自己ループと容量 0 の辺は
// 流れにもカットにも効かないので張らない。flow ごとに残りの容量を初めの容量の写しに戻す。
// 段の BFS は s から始め、t に印を付けた時点で止める (t より手前の段の頂点には印が付き終わっている)。DFS は t から s へ
// 逆にたどり (今の Library の Dinic と同じ向き)、t と同じ段の t でない頂点を見ない。流しきったあとの最後の BFS は t に届かず、
// s から届く頂点を全部たどるので、その印が最小カットの s の側になる。
// DEG_CUT を立てると、流量が s か t の容量の和 (重み付きの次数) に届いた時点で止め、カットを {s} か t 以外の全部にする
// (最後の BFS を省く)。
#include <algorithm>
#include <array>
#include <cstring>
#include <limits>
#include <vector>

namespace gomory_hu_shared {
template <bool DEG_CUT = false> struct DinicEngine {
  using i64 = long long;
  int n, s_ = 0, t_ = 0, cut_ = 0;  // cut_: 0 は印で、1 は {s}、2 は t 以外の全部
  std::vector<int> st, to, rv, lv, it, q;
  std::vector<i64> cap0, cap, deg;
  DinicEngine(int n, const std::vector<std::array<i64, 3>> &edges) : n(n), st(n + 1, 0), lv(n), it(n), q(n), deg(n, 0) {
    for (auto &e : edges)
      if (e[0] != e[1] && e[2] > 0) ++st[e[0] + 1], ++st[e[1] + 1];
    for (int v = 0; v < n; ++v) st[v + 1] += st[v];
    const int m2 = st[n];
    to.resize(m2), rv.resize(m2), cap0.resize(m2), cap.resize(m2);
    std::vector<int> fill(st.begin(), st.end() - 1);
    for (auto &e : edges)
      if (e[0] != e[1] && e[2] > 0) {
        const int u = (int)e[0], v = (int)e[1], a = fill[u]++, b = fill[v]++;
        to[a] = v, to[b] = u, rv[a] = b, rv[b] = a, cap0[a] = cap0[b] = e[2];
        deg[u] += e[2], deg[v] += e[2];
      }
  }
  // s からの段。t に届けば true。
  bool bfs(int s, int t) {
    std::fill(lv.begin(), lv.end(), -1);
    int qh = 0, qt = 0;
    lv[s] = 0, q[qt++] = s;
    while (qh < qt) {
      const int u = q[qh++], l = lv[u] + 1;
      for (int k = st[u]; k < st[u + 1]; ++k)
        if (cap[k] > 0 && lv[to[k]] < 0) {
          lv[to[k]] = l;
          if (to[k] == t) return true;
          q[qt++] = to[k];
        }
    }
    return false;
  }
  // u から s へ逆にたどり、s から u へ f まで流す。流せた量を返す。
  i64 dfs(int u, int s, i64 f) {
    if (u == s) return f;
    i64 ret = 0;
    for (int &k = it[u]; k < st[u + 1]; ++k) {
      const int w = to[k], b = rv[k];
      if (lv[w] != lv[u] - 1 || cap[b] <= 0) continue;
      const i64 d = dfs(w, s, std::min(f - ret, cap[b]));
      if (d <= 0) continue;
      cap[b] -= d, cap[k] += d, ret += d;
      if (ret == f) return ret;
    }
    lv[u] = -1;
    return ret;
  }
  i64 flow(int s, int t) {
    s_ = s, t_ = t, cut_ = 0;
    std::memcpy(cap.data(), cap0.data(), sizeof(i64) * cap.size());
    const i64 lim = std::min(deg[s], deg[t]);
    i64 f = 0;
    if (DEG_CUT && lim == 0) return cut_ = deg[s] == 0 ? 1 : 2, 0;
    while (bfs(s, t)) {
      for (int v = 0; v < n; ++v) it[v] = st[v];
      f += dfs(t, s, std::numeric_limits<i64>::max());
      if (DEG_CUT && f == lim) return cut_ = f == deg[s] ? 1 : 2, f;
    }
    return f;
  }
  bool side(int v) const { return cut_ == 0 ? lv[v] >= 0 : cut_ == 1 ? v == s_ : v != t_; }
};
}  // namespace gomory_hu_shared
