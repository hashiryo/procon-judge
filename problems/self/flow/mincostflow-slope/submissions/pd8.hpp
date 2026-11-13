#pragma once
// pd6 で、頂点ごとのポテンシャル、距離、Dijkstra で距離を付けた回と取り出した回の番号、段数と段数を付けた回の番号を、
// 32 byte の 1 つの構造体に並べたもの。Dijkstra と段数の BFS と DFS で、辺 1 本ごとに行き先の頂点を読むのが 1 か所で済み、
// 回の番号で見分けるので Dijkstra の前に距離と印を戻さなくてよい。段数の BFS は、縮約費用が 0 の弧だけ相手の弧の容量を読む。
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
    Cost p, d;           // ポテンシャルと、seen の回の Dijkstra の距離
    int seen, done;      // 距離を付けた回と、取り出した回
    int lv, lr;          // 段数と、段数を付けた回
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
  vector<int> cur, visited, path, bq;
  int dij_id = 0, round_id = 0;

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
    vs.resize(n, V{mx, INF, 0, 0, 0, 0});
    cur.resize(n);
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
              fprintf(stderr, "pd8: 残余グラフに負閉路がある\n");
              exit(1);
            }
            inq[a.to] = 1, q.push_back(a.to);
          }
        }
      }
    }
  }

  // radix heap。桶 k (k >= 1) は、最後に取り出した鍵 last との排他的論理和の最上位の bit が k - 1 の鍵。
  vector<pair<unsigned long long, int>> bucket[65];
  unsigned long long last = 0;
  int heap_size = 0;
  static int bucket_of(unsigned long long x) { return x ? 64 - __builtin_clzll(x) : 0; }
  void hpush(unsigned long long key, int v) { bucket[bucket_of(key ^ last)].push_back({key, v}), ++heap_size; }
  pair<unsigned long long, int> hpop() {
    if (bucket[0].empty()) {
      int i = 1;
      while (bucket[i].empty()) ++i;
      unsigned long long mn = bucket[i][0].first;
      for (const auto &e : bucket[i]) mn = min(mn, e.first);
      last = mn;
      for (const auto &e : bucket[i]) bucket[bucket_of(e.first ^ last)].push_back(e);
      bucket[i].clear();
    }
    --heap_size;
    auto r = bucket[0].back();
    bucket[0].pop_back();
    return r;
  }

  bool dijkstra(int s, int t) {
    ++dij_id;
    visited.clear();
    for (auto &b : bucket) b.clear();
    last = 0, heap_size = 0;
    Cost dt = INF;
    vs[s].d = 0, vs[s].seen = dij_id, hpush(0, s);
    while (heap_size) {
      const auto [key, v] = hpop();
      V &x = vs[v];
      if (x.done == dij_id || (Cost)key != x.d) continue;
      const Cost dv = x.d, pv = x.p;
      if (dv > dt) break;
      x.done = dij_id, visited.push_back(v);
      if (v == t) {
        dt = dv;
        continue;
      }
      for (int j = bg[v], e = en[v]; j < e; ++j) {
        const Arc &a = g[j];
        if (!a.cap) continue;
        V &w = vs[a.to];
        const Cost nd = dv + a.cost + pv - w.p;
        if (w.seen != dij_id || nd < w.d) w.seen = dij_id, w.d = nd, hpush(nd, a.to);
      }
    }
    if (vs[t].done != dij_id) return false;
    for (int v : visited) vs[v].p += vs[v].d - dt;
    return true;
  }

  bool levels(int s, int t) {
    ++round_id;
    bq.clear(), bq.push_back(t), vs[t].lv = 0, vs[t].lr = round_id;
    for (size_t h = 0; h < bq.size(); ++h) {
      const int v = bq[h];
      if (v == s) return true;
      const Cost pv = vs[v].p;
      const int L = vs[v].lv + 1;
      for (int j = bg[v], e = en[v]; j < e; ++j) {
        const Arc &b = g[j];
        V &u = vs[b.to];
        if (u.p - b.cost != pv || u.lr == round_id || u.done != dij_id) continue;
        if (g[b.rev].cap > 0) u.lv = L, u.lr = round_id, bq.push_back(b.to);
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
      const int want = vs[u].lv - 1, e = en[u];
      int j = cur[u];
      for (; j < e; ++j) {
        const Arc &a = g[j];
        if (!a.cap) continue;
        const V &w = vs[a.to];
        if (w.lr == round_id && w.lv == want && a.cost + pu == w.p) break;
      }
      cur[u] = j;
      if (j < e) {
        path.push_back(j), u = g[j].to;
        continue;
      }
      vs[u].lv = -1;
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
      Cap f = 0;
      while (fl + f < limit && levels(s, t)) f += block(s, t, limit - fl - f);
      fl += f, total += f * d;
      if (res.size() >= 2 && prev_d == d) res.pop_back();
      res.push_back({fl, total}), prev_d = d;
    }
    return res;
  }
};
