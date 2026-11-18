#pragma once
// Gusfield の方法の最大流の部分を、Dinic と push-relabel を組み合わせて書いたもの (gusfield.hpp の Engine)。
// 網の組み方は dinic.hpp と同じ。flow ごとに、まず dinic.hpp と同じ Dinic で流し、段の BFS と DFS で見た弧の数が
// K (n + 網の弧の数) を超えたら、そこまでの流れから hlpp.hpp と同じ push-relabel (s の残りの弧を飽和させて前流の段だけ)
// で流しきる。Dinic で流しきれば最後の BFS の印を、push-relabel に移れば t に届かない頂点の集合を最小カットにする。
// 道の短い網 (乱択、密、木に近い形) は Dinic のまま数段で終わり、道の長い網 (格子、輪、塊) は段が続く前に移る。
#include <algorithm>
#include <array>
#include <cstring>
#include <limits>
#include <vector>

namespace gomory_hu_shared {
template <int K = 4> struct HybridEngine {
  using i64 = long long;
  int n, m2, amax = -1, bmax = -1;
  bool pr = false;  // 直前の flow を push-relabel で終えた
  long long work = 0;
  std::vector<int> st, to, rv, lv, it, h, cur, anext, bnext, bprev, ahead, bhead, q;
  std::vector<i64> cap0, cap, ex;
  std::vector<char> reach;
  HybridEngine(int n, const std::vector<std::array<i64, 3>> &edges)
      : n(n), st(n + 1, 0), lv(n), it(n), h(n), cur(n), anext(n), bnext(n), bprev(n), ahead(n + 1), bhead(n + 1), q(n), ex(n),
        reach(n) {
    for (auto &e : edges)
      if (e[0] != e[1] && e[2] > 0) ++st[e[0] + 1], ++st[e[1] + 1];
    for (int v = 0; v < n; ++v) st[v + 1] += st[v];
    m2 = st[n];
    to.resize(m2), rv.resize(m2), cap0.resize(m2), cap.resize(m2);
    std::vector<int> fill(st.begin(), st.end() - 1);
    for (auto &e : edges)
      if (e[0] != e[1] && e[2] > 0) {
        const int u = (int)e[0], v = (int)e[1], a = fill[u]++, b = fill[v]++;
        to[a] = v, to[b] = u, rv[a] = b, rv[b] = a, cap0[a] = cap0[b] = e[2];
      }
  }
  // ---- Dinic ----
  bool bfs(int s, int t) {
    std::fill(lv.begin(), lv.end(), -1);
    int qh = 0, qt = 0;
    lv[s] = 0, q[qt++] = s;
    while (qh < qt) {
      const int u = q[qh++], l = lv[u] + 1;
      work += st[u + 1] - st[u];
      for (int k = st[u]; k < st[u + 1]; ++k)
        if (cap[k] > 0 && lv[to[k]] < 0) {
          lv[to[k]] = l;
          if (to[k] == t) return true;
          q[qt++] = to[k];
        }
    }
    return false;
  }
  i64 dfs(int u, int s, i64 f) {
    if (u == s) return f;
    i64 ret = 0;
    for (int &k = it[u]; k < st[u + 1]; ++k) {
      ++work;
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
  // ---- push-relabel ----
  void b_insert(int v, int d) {
    bprev[v] = -1, bnext[v] = bhead[d];
    if (bhead[d] >= 0) bprev[bhead[d]] = v;
    bhead[d] = v;
  }
  void b_erase(int v, int d) {
    if (bprev[v] >= 0) bnext[bprev[v]] = bnext[v];
    else bhead[d] = bnext[v];
    if (bnext[v] >= 0) bprev[bnext[v]] = bprev[v];
  }
  void a_push(int v, int d) { anext[v] = ahead[d], ahead[d] = v, amax = std::max(amax, d); }
  void global_relabel(int s, int t) {
    std::fill(h.begin(), h.end(), n);
    for (int d = 0; d < n; ++d) ahead[d] = bhead[d] = -1;
    amax = bmax = -1;
    int qh = 0, qt = 0;
    h[t] = 0, q[qt++] = t;
    while (qh < qt) {
      const int v = q[qh++], d = h[v] + 1;
      for (int k = st[v]; k < st[v + 1]; ++k) {
        const int w = to[k];
        if (h[w] == n && w != s && cap[rv[k]] > 0) h[w] = d, q[qt++] = w;
      }
    }
    for (int i = 1; i < qt; ++i) {
      const int v = q[i];
      b_insert(v, h[v]), bmax = h[v], cur[v] = st[v];
      if (ex[v] > 0) a_push(v, h[v]);
    }
  }
  // 今の流れから、s の残りの弧を飽和させて前流の段を回す。t に新しく届いた量を返す。
  i64 push_relabel(int s, int t) {
    std::fill(ex.begin(), ex.end(), 0);
    for (int k = st[s]; k < st[s + 1]; ++k)
      if (const i64 c = cap[k]; c > 0) ex[to[k]] += c, cap[rv[k]] += c, cap[k] = 0;
    global_relabel(s, t);
    const long long limit = 6LL * n + m2;
    long long w2 = 0;
    while (amax >= 0) {
      const int u = ahead[amax];
      if (u < 0) {
        --amax;
        continue;
      }
      ahead[amax] = anext[u];
      while (ex[u] > 0) {
        const int hu = h[u], end = st[u + 1];
        int k = cur[u];
        for (; k < end; ++k) {
          const int w = to[k];
          if (cap[k] > 0 && h[w] == hu - 1) {
            const i64 d = std::min(ex[u], cap[k]);
            if (ex[w] == 0 && w != t) a_push(w, h[w]);
            cap[k] -= d, cap[rv[k]] += d, ex[u] -= d, ex[w] += d;
            if (ex[u] == 0) break;
          }
        }
        cur[u] = k;
        if (ex[u] == 0) break;
        b_erase(u, hu);
        if (bhead[hu] < 0) {
          for (int d = hu; d <= bmax; ++d) {
            for (int v = bhead[d]; v >= 0; v = bnext[v]) h[v] = n;
            bhead[d] = ahead[d] = -1;
          }
          h[u] = n, bmax = hu - 1, amax = std::min(amax, hu - 1);
          break;
        }
        int nh = n, nk = end;
        for (int j = st[u]; j < end; ++j)
          if (cap[j] > 0 && h[to[j]] + 1 < nh) nh = h[to[j]] + 1, nk = j;
        w2 += end - st[u] + 12;
        h[u] = nh, cur[u] = nk;
        if (nh >= n) break;
        b_insert(u, nh), bmax = std::max(bmax, nh);
        if (w2 > limit) {
          w2 = 0;
          global_relabel(s, t);
          break;
        }
      }
    }
    std::fill(reach.begin(), reach.end(), 0);
    int qh = 0, qt = 0;
    reach[t] = 1, q[qt++] = t;
    while (qh < qt) {
      const int v = q[qh++];
      for (int k = st[v]; k < st[v + 1]; ++k)
        if (!reach[to[k]] && cap[rv[k]] > 0) reach[to[k]] = 1, q[qt++] = to[k];
    }
    return ex[t];
  }
  i64 flow(int s, int t) {
    std::memcpy(cap.data(), cap0.data(), sizeof(i64) * m2);
    pr = false, work = 0;
    const long long lim = (long long)K * (n + m2);
    i64 f = 0;
    while (bfs(s, t)) {
      for (int v = 0; v < n; ++v) it[v] = st[v];
      f += dfs(t, s, std::numeric_limits<i64>::max());
      if (work > lim) {
        pr = true;
        return f + push_relabel(s, t);
      }
    }
    return f;
  }
  bool side(int v) const { return pr ? !reach[v] : lv[v] >= 0; }
};
}  // namespace gomory_hu_shared
