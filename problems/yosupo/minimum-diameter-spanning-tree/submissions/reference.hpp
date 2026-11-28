#pragma once
// 最小直径全域木。最小直径の木は「絶対中心」からの最短路木になる。
//
// 1. 全頂点から Dijkstra して D[v][i] を出す。
// 2. 中心を探す。辺 (u, v, w) 上の u から x の点では、i への距離は min(D[u][i] + x, D[v][i] + w - x)。
//    i が u 側を好むのは x <= (D[v][i] + w - D[u][i]) / 2 のときなので、この値で並べると
//    各 x について「u 側を好む頂点は後ろの一区間」になる。区間の切れ目ごとに
//    両側の最大が釣り合う x を取り、最良の点を選ぶ。半分の x が出るので 2 倍で持つ。
// 3. 中心が辺の上なら、その辺を入れて、各頂点を近い側の最短路木につなぐ。
//    中心が頂点なら、その頂点からの最短路木にする。
// 4. 直径は作った木の上で 2 回の幅優先で出し直す (整数になる)。
#include <algorithm>
#include <queue>
#include <utility>
#include <vector>

struct Solver {
  int n, m;
  std::vector<int> ea, eb;
  std::vector<long long> ec;
  std::vector<int> chosen;
  long long diam = 0;

  Solver(int n_, std::vector<int> a, std::vector<int> b, std::vector<long long> c)
      : n(n_), m((int)a.size()), ea(std::move(a)), eb(std::move(b)), ec(std::move(c)) {}

  // s からの距離と、入ってきた辺。
  void dijkstra(int s, std::vector<long long> &d, std::vector<int> &pe,
                const std::vector<int> &head, const std::vector<int> &nxt,
                const std::vector<int> &to, const std::vector<int> &eid,
                const std::vector<long long> &w) const {
    const long long INF = (long long)1 << 62;
    d.assign(n, INF);
    pe.assign(n, -1);
    using P = std::pair<long long, int>;
    std::priority_queue<P, std::vector<P>, std::greater<P>> pq;
    d[s] = 0;
    pq.emplace(0, s);
    while (!pq.empty()) {
      auto [dv, v] = pq.top();
      pq.pop();
      if (dv != d[v]) continue;
      for (int e = head[v]; e != -1; e = nxt[e]) {
        const int u = to[e];
        if (d[v] + w[e] < d[u]) {
          d[u] = d[v] + w[e];
          pe[u] = eid[e];
          pq.emplace(d[u], u);
        }
      }
    }
  }

  void run() {
    if (n == 1) {
      chosen.clear();
      diam = 0;
      return;
    }
    // 隣接 (辺の番号も持つ)
    std::vector<int> head(n, -1), nxt(2 * m), to(2 * m), eid(2 * m);
    std::vector<long long> w(2 * m);
    for (int i = 0; i < m; ++i) {
      to[2 * i] = eb[i], eid[2 * i] = i, w[2 * i] = ec[i], nxt[2 * i] = head[ea[i]], head[ea[i]] = 2 * i;
      to[2 * i + 1] = ea[i], eid[2 * i + 1] = i, w[2 * i + 1] = ec[i], nxt[2 * i + 1] = head[eb[i]];
      head[eb[i]] = 2 * i + 1;
    }
    const long long INF = (long long)1 << 60;
    std::vector<std::vector<long long>> D(n);
    for (int v = 0; v < n; ++v) {
      std::vector<int> pe;
      dijkstra(v, D[v], pe, head, nxt, to, eid, w);
    }
    // 中心を探す。ecc2 は偏心の 2 倍。
    long long best2 = INF;
    int bestEdge = -1, bestX2 = 0, bestV = 0;
    for (int v = 0; v < n; ++v) {
      long long e = 0;
      for (int i = 0; i < n; ++i) e = std::max(e, D[v][i]);
      if (2 * e < best2) best2 = 2 * e, bestEdge = -1, bestV = v, bestX2 = 0;
    }
    std::vector<int> ord(n);
    for (int e = 0; e < m; ++e) {
      const int u = ea[e], v = eb[e];
      const long long ww = ec[e];
      for (int i = 0; i < n; ++i) ord[i] = i;
      // t_i = D[v][i] + w - D[u][i] の昇順
      std::sort(ord.begin(), ord.end(), [&](int x, int y) {
        return (D[v][x] + ww - D[u][x]) < (D[v][y] + ww - D[u][y]);
      });
      std::vector<long long> sufA(n + 1), preC(n + 1);
      sufA[n] = -INF;
      for (int k = n - 1; k >= 0; --k) sufA[k] = std::max(sufA[k + 1], D[u][ord[k]]);
      preC[0] = -INF;
      for (int k = 0; k < n; ++k) preC[k + 1] = std::max(preC[k], D[v][ord[k]] + ww);
      for (int k = 0; k <= n; ++k) {
        const long long A = sufA[k], C = preC[k];
        long long x2;
        if (A <= -INF) x2 = 0;              // u 側を使う頂点が無い
        else if (C <= -INF) x2 = 2 * ww;    // v 側を使う頂点が無い
        else x2 = C - A;
        x2 = std::max(0LL, std::min(2 * ww, x2));
        long long e2 = -INF;
        if (A > -INF) e2 = std::max(e2, 2 * A + x2);
        if (C > -INF) e2 = std::max(e2, 2 * C - x2);
        if (e2 < best2) best2 = e2, bestEdge = e, bestX2 = (int)x2;
      }
    }
    // 木を組む
    std::vector<int> pe;
    if (bestEdge == -1) {
      std::vector<long long> d;
      dijkstra(bestV, d, pe, head, nxt, to, eid, w);
      chosen.clear();
      for (int i = 0; i < n; ++i)
        if (i != bestV) chosen.push_back(pe[i]);
    } else {
      const int u = ea[bestEdge], v = eb[bestEdge];
      const long long ww = ec[bestEdge];
      std::vector<long long> du(n), dv2(n);
      std::vector<int> pu(n), pv(n);
      dijkstra(u, du, pu, head, nxt, to, eid, w);
      dijkstra(v, dv2, pv, head, nxt, to, eid, w);
      chosen.clear();
      chosen.push_back(bestEdge);
      for (int i = 0; i < n; ++i) {
        if (i == u || i == v) continue;
        const bool left = 2 * du[i] + bestX2 <= 2 * dv2[i] + 2 * ww - bestX2;
        chosen.push_back(left ? pu[i] : pv[i]);
      }
    }
    // 直径を木の上で出し直す
    std::vector<int> th(n, -1), tn(2 * (n - 1)), tt(2 * (n - 1));
    std::vector<long long> tw(2 * (n - 1));
    for (size_t i = 0; i < chosen.size(); ++i) {
      const int e = chosen[i], a = ea[e], b = eb[e];
      tt[2 * i] = b, tw[2 * i] = ec[e], tn[2 * i] = th[a], th[a] = 2 * i;
      tt[2 * i + 1] = a, tw[2 * i + 1] = ec[e], tn[2 * i + 1] = th[b], th[b] = 2 * i + 1;
    }
    auto far = [&](int s, long long &out) {
      std::vector<long long> d(n, -1);
      std::vector<int> q{s};
      d[s] = 0;
      int p = s;
      for (size_t qi = 0; qi < q.size(); ++qi) {
        const int x = q[qi];
        if (d[x] > d[p]) p = x;
        for (int e = th[x]; e != -1; e = tn[e])
          if (d[tt[e]] < 0) d[tt[e]] = d[x] + tw[e], q.push_back(tt[e]);
      }
      out = d[p];
      return p;
    };
    long long tmp;
    const int p0 = far(0, tmp);
    far(p0, diam);
  }

  long long diameter() const { return diam; }
  const std::vector<int> &edges() const { return chosen; }
};
