#pragma once
// Gusfield の方法の最大流の部分 (gusfield.hpp の Engine) の 2 回目。Dinic と push-relabel のどちらで流しても同じ最小
// カットを返すようにして、方式を flow ごとに選べるようにした。カットの取り方は次のとおり (どれも最小カットで、最大流の
// 選び方によらず 1 つに決まる)。
//   - 流量が s の重み付きの次数に等しければ {s}
//   - そうでなく、t の重み付きの次数に等しければ t 以外の全部
//   - どちらでもなければ、残余グラフで s から届く頂点の集合 S* (どの最大流でも同じ。s の側の最小のもの)
// Dinic は最後の BFS (t に届かない) がそのまま S* になる。push-relabel は前流の段 (t へ流す) のあと、3 つ目の場合だけ
// 余りを s へ戻す段を回してから、s から BFS で S* を求める。
// push-relabel の global relabel は、余りのある頂点に全部高さが付いた段で BFS を止め、まだ付いていない頂点を次の段の高さに
// する (LAZY)。止めた段を L とすると、L までの頂点には正しい距離が付き、残りに L + 1 を付けても、残余の弧 u → w について
// h[u] ≤ h[w] + 1 が崩れない (L 未満の頂点の隣はすべて L 以下で、BFS が付け終えている)。
// MODE は、0 が Dinic だけ、1 が push-relabel だけ、2 が flow ごとの仕事量 (見た弧の数と頂点の数) の指数移動平均の小さい
// ほうを選び、R 回に 1 回はもう一方で流して測り直すもの。TOUCH を立てると、flow ごとに全部の弧の容量を写し直さず、容量を
// 変えた弧だけを覚えて戻す。CACHE を立てると、push-relabel の最初の高さに、元のグラフで t から測った距離を使う (t が前の
// flow と同じなら測り直さない)。s の弧を飽和させて s を高さ n に置くと、残りの残余の弧はどれも元のグラフにある弧なので、
// 元のグラフの距離は高さの付け方の条件 (残余の弧 u → w について h[u] ≤ h[w] + 1) を満たす。Gusfield の t は、カットが {s}
// のままなら根のまま続くので、最初の global relabel をほとんど省ける。CACHE が 1 なら t が変わるたびに元のグラフ全体を BFS で
// 測り、2 なら BFS を途中で止めて状態を残し、余りのある頂点 (s の隣) に距離が付くところまでだけ進める。まだ印の無い頂点の
// 高さは、待ち行列の先頭の距離 x に 1 を足したもの (取り出し終えたのは距離 x 未満の頂点なので、印の無い頂点の隣の印の
// ある頂点は距離 x 以上で、印のある頂点は距離 x + 1 以下。どちらの向きの弧でも h[u] ≤ h[w] + 1 が崩れない)。
#include <algorithm>
#include <array>
#include <cstring>
#include <limits>
#include <vector>

namespace gomory_hu_shared {
template <int MODE, bool LAZY = true, int R = 16, bool TOUCH = false, int CACHE = 0> struct Engine2 {
  using i64 = long long;
  int n, m2, s_ = 0, t_ = 0, cut_ = 0, amax = -1, bmax = -1;  // cut_: 0 は S* (lv)、1 は {s}、2 は t 以外の全部
  long long work = 0, cnt = 0;
  double ema[2] = {0, 0};
  std::vector<int> st, to, rv, lv, it, h, cur, anext, bnext, bprev, ahead, bhead, q, touched;
  std::vector<i64> cap0, cap, ex, deg;
  std::vector<char> mark;
  std::vector<int> dist, dq;
  int dist_t = -1, dqh = 0, dqt = 0;
  void touch(int k) {
    if (TOUCH && !mark[k]) mark[k] = 1, touched.push_back(k);
  }
  Engine2(int n, const std::vector<std::array<i64, 3>> &edges)
      : n(n), st(n + 1, 0), lv(n), it(n), h(n), cur(n), anext(n), bnext(n), bprev(n), ahead(n + 2), bhead(n + 2), q(n), ex(n),
        deg(n, 0) {
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
        deg[u] += e[2], deg[v] += e[2];
      }
    if (TOUCH) cap = cap0, mark.assign(m2, 0);
  }
  // ---- Dinic ----
  bool bfs(int s, int t) {
    std::fill(lv.begin(), lv.end(), -1);
    int qh = 0, qt = 0;
    lv[s] = 0, q[qt++] = s;
    while (qh < qt) {
      const int u = q[qh++], l = lv[u] + 1;
      work += st[u + 1] - st[u] + 1;
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
      touch(b), cap[b] -= d, cap[k] += d, ret += d;
      if (ret == f) return ret;
    }
    lv[u] = -1;
    return ret;
  }
  i64 dinic(int s, int t) {
    const i64 ds = deg[s], dt = deg[t];
    i64 f = 0;
    for (;;) {
      if (f == ds) return cut_ = 1, f;
      if (f == dt) return cut_ = 2, f;
      if (!bfs(s, t)) return cut_ = 0, f;
      for (int v = 0; v < n; ++v) it[v] = st[v];
      f += dfs(t, s, std::numeric_limits<i64>::max());
    }
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
  // 高さを sink からの距離に付け直す。excl は高さ n に置いて使わない。
  void global_relabel(int sink, int excl) {
    std::fill(h.begin(), h.end(), -1);
    for (int d = 0; d <= n; ++d) ahead[d] = bhead[d] = -1;
    amax = bmax = -1;
    int need = 0;
    for (int v = 0; v < n; ++v) need += ex[v] > 0 && v != sink && v != excl;
    int qh = 0, qt = 0, stop = LAZY && need == 0 ? 0 : n;  // stop: この高さの頂点を取り出したら止める
    h[sink] = 0, q[qt++] = sink, h[excl] = n;
    while (qh < qt) {
      const int v = q[qh++];
      if (LAZY && h[v] >= stop) {
        --qh;
        break;
      }
      const int d = h[v] + 1;
      work += st[v + 1] - st[v] + 1;
      for (int k = st[v]; k < st[v + 1]; ++k) {
        const int w = to[k];
        if (h[w] < 0 && cap[rv[k]] > 0) {
          h[w] = d, q[qt++] = w;
          if (ex[w] > 0 && --need == 0) stop = d;
        }
      }
    }
    if (LAZY && stop < n) {
      // 取り出していない頂点は高さ stop (付いている)。付いていない頂点は stop + 1。
      for (int v = 0; v < n; ++v)
        if (h[v] < 0) h[v] = stop + 1;
      work += n;
    } else {
      for (int v = 0; v < n; ++v)
        if (h[v] < 0) h[v] = n;
    }
    for (int v = 0; v < n; ++v)
      if (v != sink && h[v] < n) {
        b_insert(v, h[v]), bmax = std::max(bmax, h[v]), cur[v] = st[v];
        if (ex[v] > 0) a_push(v, h[v]);
      }
    work += n;
  }
  // 今の余りを sink へ押し出す (excl には押さない)。sink に届かない余りは残る。
  void discharge(int sink, int excl) {
    global_relabel(sink, excl);
    discharge_from_heights(sink, excl);
  }
  // 今の高さと表のまま、余りを sink へ押し出す。
  void discharge_from_heights(int sink, int excl) {
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
            if (ex[w] == 0 && w != sink) a_push(w, h[w]);
            touch(k), cap[k] -= d, cap[rv[k]] += d, ex[u] -= d, ex[w] += d;
            if (ex[u] == 0) break;
          }
        }
        work += k - cur[u] + 1;
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
        w2 += end - st[u] + 12, work += end - st[u] + 1;
        h[u] = nh, cur[u] = nk;
        if (nh >= n) break;
        b_insert(u, nh), bmax = std::max(bmax, nh);
        if (w2 > limit) {
          w2 = 0;
          global_relabel(sink, excl);
          break;
        }
      }
    }
  }
  // 元のグラフで t からの距離を dist に測る (届かない頂点は n)。
  void measure(int t) {
    dist.assign(n, n);
    int qh = 0, qt = 0;
    dist[t] = 0, q[qt++] = t;
    while (qh < qt) {
      const int v = q[qh++];
      for (int k = st[v]; k < st[v + 1]; ++k)
        if (dist[to[k]] == n) dist[to[k]] = dist[v] + 1, q[qt++] = to[k];
    }
    dist_t = t;
  }
  i64 push_relabel(int s, int t) {
    std::fill(ex.begin(), ex.end(), 0);
    work += n;
    for (int k = st[s]; k < st[s + 1]; ++k) {
      const i64 c = cap[k];
      touch(k), ex[to[k]] += c, cap[rv[k]] += c, cap[k] = 0;
    }
    if constexpr (CACHE != 0) {
      int rest = n;
      if constexpr (CACHE == 1) {
        if (dist_t != t) measure(t);
      } else {
        if (dist_t != t) {
          dist.assign(n, -1), dq.resize(n);
          dist[t] = 0, dq[0] = t, dqh = 0, dqt = 1, dist_t = t;
        }
        // 距離の要る頂点 (s の隣で、まだ印の無いもの) に -2 を付けて数える。
        int need = 0;
        for (int k = st[s]; k < st[s + 1]; ++k)
          if (dist[to[k]] == -1) dist[to[k]] = -2, ++need;
        while (need > 0 && dqh < dqt) {
          const int v = dq[dqh++];
          for (int k = st[v]; k < st[v + 1]; ++k)
            if (const int w = to[k]; dist[w] < 0) {
              need -= dist[w] == -2;
              dist[w] = dist[v] + 1, dq[dqt++] = w;
            }
        }
        if (dqh < dqt) rest = dist[dq[dqh]] + 1;
      }
      // 元のグラフの距離を高さにし、高さごとの表を作る (global relabel の代わり)。
      for (int d = 0; d <= n; ++d) ahead[d] = bhead[d] = -1;
      amax = bmax = -1;
      for (int v = 0; v < n; ++v) {
        h[v] = v == s ? n : dist[v] >= 0 ? dist[v] : rest;
        if (v != t && h[v] < n) {
          b_insert(v, h[v]), bmax = std::max(bmax, h[v]), cur[v] = st[v];
          if (ex[v] > 0) a_push(v, h[v]);
        }
      }
      discharge_from_heights(t, s);
    } else discharge(t, s);
    const i64 f = ex[t];
    if (f == deg[s]) return cut_ = 1, f;
    if (f == deg[t]) return cut_ = 2, f;
    // 余りを s へ戻し、s から届く頂点を求める。
    ex[t] = 0;
    discharge(s, t);
    std::fill(lv.begin(), lv.end(), -1);
    int qh = 0, qt = 0;
    lv[s] = 0, q[qt++] = s;
    while (qh < qt) {
      const int u = q[qh++];
      work += st[u + 1] - st[u] + 1;
      for (int k = st[u]; k < st[u + 1]; ++k)
        if (cap[k] > 0 && lv[to[k]] < 0) lv[to[k]] = 0, q[qt++] = to[k];
    }
    return cut_ = 0, f;
  }
  i64 flow(int s, int t) {
    s_ = s, t_ = t;
    if (TOUCH) {
      for (int k : touched) cap[k] = cap0[k], cap[rv[k]] = cap0[rv[k]], mark[k] = 0;
      touched.clear();
    } else std::memcpy(cap.data(), cap0.data(), sizeof(i64) * m2);
    if constexpr (MODE == 0) return dinic(s, t);
    else if constexpr (MODE == 1) return push_relabel(s, t);
    else {
      int e = ema[1] < ema[0] ? 1 : 0;
      if (++cnt % R == 0) e ^= 1;
      if (cnt <= 2) e = (int)cnt - 1;  // 初めの 2 回で両方を測る
      work = 0;
      const i64 f = e ? push_relabel(s, t) : dinic(s, t);
      const double w = (double)work + m2 / 4 + n;  // 容量の写しの分を足す
      ema[e] = cnt <= 2 ? w : ema[e] + (w - ema[e]) / 8;
      return f;
    }
  }
  bool side(int v) const { return cut_ == 0 ? lv[v] >= 0 : cut_ == 1 ? v == s_ : v != t_; }
};
}  // namespace gomory_hu_shared
