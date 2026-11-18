#pragma once
// Gusfield の方法の最大流の部分を、網を 1 回だけ組む push-relabel で書いたもの (gusfield.hpp の Engine)。
// 網の組み方は dinic.hpp と同じ (無向辺は互いを逆向きとする容量 c の弧の組、自己ループと容量 0 の辺は張らない)。
// flow ごとに残りの容量を写しに戻し、s の弧をすべて飽和させてから、highest label (同じ高さの中は後入れ先出し) で前流の
// 段だけを回す。高さは t からの距離で始め (global relabel)、relabel で見た弧の数が 6n + 網の弧の数を超えるたびに付け直す。
// 高さ d の頂点がいなくなったら、d より高い頂点の高さを n にする (gap)。余りを s へ戻す段は回さない。
// 最小カットは、前流の段のあとの残余グラフで t に届かない頂点の集合 (s を含む) で、t からの逆向きの BFS で求める。
#include <algorithm>
#include <array>
#include <cstring>
#include <vector>

namespace gomory_hu_shared {
struct HlppEngine {
  using i64 = long long;
  int n, m2, amax = -1, bmax = -1;
  std::vector<int> st, to, rv, h, cur, anext, bnext, bprev, ahead, bhead, q;
  std::vector<i64> cap0, cap, ex;
  std::vector<char> reach;
  HlppEngine(int n, const std::vector<std::array<i64, 3>> &edges)
      : n(n), st(n + 1, 0), h(n), cur(n), anext(n), bnext(n), bprev(n), ahead(n + 1), bhead(n + 1), q(n), ex(n), reach(n) {
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
  // 高さを t からの距離に付け直し、高さごとの表を作り直す。s と、t に届かない頂点は高さ n。
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
  i64 flow(int s, int t) {
    std::memcpy(cap.data(), cap0.data(), sizeof(i64) * m2);
    std::fill(ex.begin(), ex.end(), 0);
    for (int k = st[s]; k < st[s + 1]; ++k) {
      const i64 c = cap[k];
      ex[to[k]] += c, cap[rv[k]] += c, cap[k] = 0;
    }
    global_relabel(s, t);
    const long long limit = 6LL * n + m2;
    long long work = 0;
    while (amax >= 0) {
      const int u = ahead[amax];
      if (u < 0) {
        --amax;
        continue;
      }
      ahead[amax] = anext[u];
      // u の余りを押し出す。押しきれなければ relabel して続ける。
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
        // relabel。高さ hu の頂点が u だけなら gap で hu 以上を全部 n にする。
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
        work += end - st[u] + 12;
        h[u] = nh, cur[u] = nk;
        if (nh >= n) break;
        b_insert(u, nh), bmax = std::max(bmax, nh);
        if (work > limit) {
          work = 0;
          a_push(u, nh);
          global_relabel(s, t);
          break;
        }
      }
    }
    // t に届く頂点を逆向きにたどる。
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
  bool side(int v) const { return !reach[v]; }
};
}  // namespace gomory_hu_shared
