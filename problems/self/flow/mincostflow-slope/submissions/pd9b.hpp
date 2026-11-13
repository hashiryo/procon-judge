#pragma once
// pd9 の radix heap を、ACL と同じ二分ヒープ (縮約費用 0 の弧で着いた頂点は二分ヒープを通さずに積む que_min) に戻したもの。
// x64 の EPYC 9V74 で、区間の詰め込みと乱択の網では ACL の Dijkstra が radix heap より 1〜2 割速かったため。以下は pd9 の説明。
// pd6 に 2 つの手を足したもの。1 つ目は、Dijkstra の最短路木の s から t への道に先に流してから、段数の BFS と Dinic に
// 移ること。距離がみな違う網では、BFS は「同じ距離の道がもう無い」ことを確かめる 1 回で済む (pd6 は、道を見つける BFS と
// 確かめる BFS の 2 回)。2 つ目は、段数の BFS と DFS で、行き先の頂点のポテンシャルで縮約費用が 0 かを先に確かめ、0 の
// 弧だけ印と相手の弧の容量を読むこと (pd8 と同じ順)。頂点ごとのポテンシャルと距離は 16 byte の組のまま (pd8 の 32 byte
// の構造体は、頂点の多い網で Dijkstra がキャッシュから外れやすくなった)。
#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <limits>
#include <utility>
#include <vector>

struct Solver {
  using Cap = i64;
  using Cost = i64;
  static constexpr Cost INF = numeric_limits<Cost>::max();
  struct Arc {
    int to, rev;
    Cap cap;
    Cost cost;
  };
  struct V {
    Cost p, d;  // ポテンシャルと、今の Dijkstra の距離
  };
  int n;
  vector<int> efrom, eto;
  vector<Cap> ecap;
  vector<Cost> ecost;
  int built_m = 0, built_n = 0;
  vector<int> bg, en, room;  // 頂点ごとの区画 [bg, en) と、区画の大きさ
  vector<int> aid, pos;      // 弧ごとの「辺の番号の 2 倍と向き」と、その逆引き
  vector<Arc> g;
  vector<V> vs;
  vector<int> prv, cur, touched, visited, path, lvl, lr, bq;
  vector<char> vis;
  int round_id = 0;

  explicit Solver(int n) : n(n) {}
  int add_vertex() { return n++; }
  int add_edge(int from, int to_, Cap c, Cost w) {
    efrom.push_back(from), eto.push_back(to_), ecap.push_back(c), ecost.push_back(w);
    return (int)efrom.size() - 1;
  }
  i64 flow(int i) const { return i < built_m ? ecap[i] - g[pos[2 * i]].cap : 0; }

  // 頂点 v の区画を大きさ r にして配列の末尾へ移す。
  void relocate(int v, int r) {
    const int ns = g.size(), d = en[v] - bg[v];
    g.resize(ns + r), aid.resize(ns + r);
    for (int k = 0; k < d; ++k) g[ns + k] = g[bg[v] + k], aid[ns + k] = aid[bg[v] + k], pos[aid[ns + k]] = ns + k;
    for (int k = 0; k < d; ++k) {
      const int a = ns + k, b = pos[aid[a] ^ 1];
      if (b >= 0) g[a].rev = b, g[b].rev = a;
    }
    bg[v] = ns, en[v] = ns + d, room[v] = r;
  }
  int place(int v) {
    if (en[v] - bg[v] == room[v]) relocate(v, max(4, 2 * room[v]));
    return en[v]++;
  }

  void build() {
    const int m = efrom.size();
    if (m == built_m && n == built_n) return;
    pos.resize(2 * m, -1);
    if (built_m == 0) {
      bg.assign(n + 1, 0);
      for (int i = 0; i < m; ++i) ++bg[efrom[i] + 1], ++bg[eto[i] + 1];
      for (int v = 0; v < n; ++v) bg[v + 1] += bg[v];
      en.assign(bg.begin(), bg.end() - 1), bg.pop_back(), room.resize(n);
      for (int v = 0; v < n; ++v) room[v] = (v + 1 < n ? bg[v + 1] : 2 * m) - bg[v];
      g.resize(2 * m), aid.resize(2 * m);
      for (int i = 0; i < m; ++i) {
        const int a = en[efrom[i]]++, b = en[eto[i]]++;
        g[a] = {eto[i], b, ecap[i], ecost[i]}, g[b] = {efrom[i], a, 0, -ecost[i]};
        aid[a] = 2 * i, aid[b] = 2 * i + 1, pos[2 * i] = a, pos[2 * i + 1] = b;
      }
    } else {
      bg.resize(n, (int)g.size()), en.resize(n, (int)g.size()), room.resize(n, 0);
      for (int i = built_m; i < m; ++i) {
        const int a = place(efrom[i]);
        g[a] = {eto[i], -1, ecap[i], ecost[i]}, aid[a] = 2 * i, pos[2 * i] = a;
        const int b = place(eto[i]);
        g[b] = {efrom[i], -1, 0, -ecost[i]}, aid[b] = 2 * i + 1, pos[2 * i + 1] = b;
        g[pos[2 * i]].rev = b, g[b].rev = pos[2 * i];
      }
    }
    Cost mx = 0;
    for (const V &x : vs) mx = max(mx, x.p);
    vs.resize(n, V{mx, INF});
    prv.resize(n), cur.resize(n), vis.resize(n, 0), lvl.resize(n), lr.resize(n, 0);
    bool bad = false;
    for (int i = built_m; i < m && !bad; ++i)
      if (ecap[i] > 0 && vs[efrom[i]].p + ecost[i] < vs[eto[i]].p) bad = true;
    built_m = m, built_n = n;
    if (bad) repair();
  }

  void repair() {
    vector<int> indeg(n, 0), order;
    order.reserve(n);
    for (int u = 0; u < n; ++u)
      for (int j = bg[u]; j < en[u]; ++j)
        if (g[j].cap > 0) ++indeg[g[j].to];
    for (int v = 0; v < n; ++v)
      if (!indeg[v]) order.push_back(v);
    for (size_t k = 0; k < order.size(); ++k) {
      const int u = order[k];
      for (int j = bg[u]; j < en[u]; ++j)
        if (g[j].cap > 0 && !--indeg[g[j].to]) order.push_back(g[j].to);
    }
    if ((int)order.size() == n) {
      for (int u : order)
        for (int j = bg[u]; j < en[u]; ++j)
          if (g[j].cap > 0 && vs[u].p + g[j].cost < vs[g[j].to].p) vs[g[j].to].p = vs[u].p + g[j].cost;
      return;
    }
    vector<int> q(n), cnt(n, 0);
    vector<char> inq(n, 1);
    for (int v = 0; v < n; ++v) q[v] = v;
    for (size_t head = 0; head < q.size(); ++head) {
      const int u = q[head];
      inq[u] = 0;
      for (int j = bg[u]; j < en[u]; ++j) {
        const Arc &a = g[j];
        if (a.cap > 0 && vs[u].p + a.cost < vs[a.to].p) {
          vs[a.to].p = vs[u].p + a.cost;
          if (!inq[a.to]) {
            if (++cnt[a.to] > n) {
              fprintf(stderr, "pd9b: 残余グラフに負閉路がある\n");
              exit(1);
            }
            inq[a.to] = 1, q.push_back(a.to);
          }
        }
      }
    }
  }

  struct Q {
    Cost key;
    int to;
    bool operator<(Q r) const { return key > r.key; }
  };
  vector<int> que_min;
  vector<Q> que;

  bool dijkstra(int s, int t) {
    for (int v : touched) vs[v].d = INF;
    for (int v : visited) vis[v] = 0;
    touched.clear(), visited.clear(), que_min.clear(), que.clear();
    size_t heap_r = 0;
    Cost dt = INF;
    vs[s].d = 0, touched.push_back(s), que_min.push_back(s);
    while (!que_min.empty() || !que.empty()) {
      int v;
      if (!que_min.empty()) {
        v = que_min.back();
        que_min.pop_back();
      } else {
        while (heap_r < que.size()) ++heap_r, push_heap(que.begin(), que.begin() + heap_r);
        v = que.front().to;
        pop_heap(que.begin(), que.end());
        que.pop_back();
        --heap_r;
      }
      if (vis[v]) continue;
      const Cost dv = vs[v].d, pv = vs[v].p;
      if (dv > dt) break;
      vis[v] = 1, visited.push_back(v);
      if (v == t) {
        dt = dv;
        continue;
      }
      for (int j = bg[v], e = en[v]; j < e; ++j) {
        const Arc &a = g[j];
        if (!a.cap) continue;
        V &w = vs[a.to];
        const Cost nd = dv + a.cost + pv - w.p;
        if (nd < w.d) {
          if (w.d == INF) touched.push_back(a.to);
          w.d = nd, prv[a.to] = j;
          if (nd == dv) que_min.push_back(a.to);
          else que.push_back(Q{nd, a.to});
        }
      }
    }
    if (!vis[t]) return false;
    for (int v : visited) vs[v].p += vs[v].d - dt;
    return true;
  }

  bool levels(int s, int t) {
    ++round_id;
    bq.clear(), bq.push_back(t), lvl[t] = 0, lr[t] = round_id;
    for (size_t h = 0; h < bq.size(); ++h) {
      const int v = bq[h];
      if (v == s) return true;
      const Cost pv = vs[v].p;
      const int L = lvl[v] + 1;
      for (int j = bg[v], e = en[v]; j < e; ++j) {
        const Arc &b = g[j];
        const int u = b.to;
        if (vs[u].p - b.cost != pv || lr[u] == round_id || !vis[u]) continue;
        if (g[b.rev].cap > 0) lvl[u] = L, lr[u] = round_id, bq.push_back(u);
      }
    }
    return false;
  }

  Cap block(int s, int t, Cap limit) {
    for (int v : bq) cur[v] = bg[v];
    Cap pushed = 0;
    path.clear();
    int u = s;
    while (true) {
      if (u == t) {
        Cap f = limit - pushed;
        for (int j : path) f = min(f, g[j].cap);
        for (int j : path) g[j].cap -= f, g[g[j].rev].cap += f;
        pushed += f;
        if (pushed == limit) break;
        size_t k = 0;
        while (g[path[k]].cap > 0) ++k;
        path.resize(k);
        u = k ? g[path[k - 1]].to : s;
        continue;
      }
      const Cost pu = vs[u].p;
      const int want = lvl[u] - 1, e = en[u];
      int j = cur[u];
      for (; j < e; ++j) {
        const Arc &a = g[j];
        if (a.cap > 0 && a.cost + pu == vs[a.to].p && lr[a.to] == round_id && lvl[a.to] == want) break;
      }
      cur[u] = j;
      if (j < e) {
        path.push_back(j), u = g[j].to;
        continue;
      }
      lvl[u] = -1;
      if (u == s) break;
      const int b = path.back();
      path.pop_back();
      u = g[g[b].rev].to;
      ++cur[u];
    }
    return pushed;
  }

  vector<pair<Cap, Cost>> slope(int s, int t, Cap limit) {
    build();
    vector<pair<Cap, Cost>> res = {{0, 0}};
    if (s == t) return res;
    Cap fl = 0;
    Cost total = 0, prev_d = 0;
    while (fl < limit && dijkstra(s, t)) {
      const Cost d = vs[t].p - vs[s].p;
      // 最短路木の道に流してから、残りを縮約費用 0 の網で流す。
      Cap f = limit - fl;
      for (int v = t; v != s; v = g[g[prv[v]].rev].to) f = min(f, g[prv[v]].cap);
      for (int v = t; v != s; v = g[g[prv[v]].rev].to) g[prv[v]].cap -= f, g[g[prv[v]].rev].cap += f;
      while (fl + f < limit && levels(s, t)) f += block(s, t, limit - fl - f);
      fl += f, total += f * d;
      if (res.size() >= 2 && prev_d == d) res.pop_back();
      res.push_back({fl, total}), prev_d = d;
    }
    return res;
  }
};
