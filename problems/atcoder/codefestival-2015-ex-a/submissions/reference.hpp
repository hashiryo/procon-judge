#pragma once
// 期待出力を作る参照実装。ライブラリを使わずに書いた (2026-11-18)。1 から N へ単位容量の Dinic で流し、残余グラフの強連結
// 成分を Tarjan の方法 (再帰しない形) で求め、成分を縮めた DAG で、成分ごとに届く成分の集合を bitset で持つ。
//
// 最小カットの S は、残余グラフで閉じていて (S から出る残余の弧が無い) s = 1 を含み t = N を含まない集合と一致する。s から
// 届く頂点の集合を A、t へ届く頂点の集合を B とする。c と d を両方含む S があるのは、c からも d からも t へ届かないとき
// (s、c、d から届く頂点を S にする)。両方含まない S があるのは、s から c にも d にも届かないとき (c、d、t へ届く頂点を T に
// する)。c を含み d を含まない S があるのは、c から t へ届かず、s からも c からも d へ届かないとき。小さい入力では
// brute.hpp (S を全部試す) と、gen.py の seed 1000 から 2999 で突き合わせてある。
#include <algorithm>
#include <vector>

struct Solver {
  int n;
  const vector<array<int, 2>> &roads, &qs;
  Solver(int n, const vector<array<int, 2>> &roads, const vector<array<int, 2>> &qs) : n(n), roads(roads), qs(qs) {}
  struct Arc {
    int to, cap, rev;
  };
  vector<vector<Arc>> g;
  vector<int> lv, itr;
  int dfs(int v, int t, int f) {
    if (v == t) return f;
    for (int &i = itr[v]; i < (int)g[v].size(); ++i) {
      Arc &a = g[v][i];
      if (a.cap <= 0 || lv[a.to] != lv[v] + 1) continue;
      if (int d = dfs(a.to, t, min(f, a.cap)); d > 0) return a.cap -= d, g[a.to][a.rev].cap += d, d;
    }
    return 0;
  }
  vector<int> run() {
    const int s = 0, t = n - 1;
    g.assign(n, {});
    for (auto &[a, b] : roads) {
      g[a].push_back({b, 1, (int)g[b].size()});
      g[b].push_back({a, 0, (int)g[a].size() - 1});
    }
    for (;;) {
      lv.assign(n, -1), lv[s] = 0;
      vector<int> q = {s};
      for (size_t h = 0; h < q.size(); ++h)
        for (auto &a : g[q[h]])
          if (a.cap > 0 && lv[a.to] < 0) lv[a.to] = lv[q[h]] + 1, q.push_back(a.to);
      if (lv[t] < 0) break;
      itr.assign(n, 0);
      while (dfs(s, t, 1 << 30) > 0) {}
    }
    // 残余グラフの強連結成分。Tarjan の方法は、行き先の成分ほど先に番号が付く (弧 u → v なら comp[u] >= comp[v])。
    vector<int> ord(n, -1), low(n), comp(n, -1), it(n, 0), stk, call;
    int cnt = 0, K = 0;
    for (int r = 0; r < n; ++r) {
      if (ord[r] >= 0) continue;
      ord[r] = low[r] = cnt++, stk.push_back(r), call.push_back(r);
      while (!call.empty()) {
        const int v = call.back();
        if (it[v] < (int)g[v].size()) {
          const Arc &a = g[v][it[v]++];
          if (a.cap <= 0) continue;
          if (ord[a.to] < 0) ord[a.to] = low[a.to] = cnt++, stk.push_back(a.to), call.push_back(a.to);
          else if (comp[a.to] < 0) low[v] = min(low[v], ord[a.to]);
          continue;
        }
        call.pop_back();
        if (!call.empty()) low[call.back()] = min(low[call.back()], low[v]);
        if (low[v] == ord[v]) {
          for (int w = -1; w != v; stk.pop_back()) comp[w = stk.back()] = K;
          ++K;
        }
      }
    }
    const int W = (K + 63) / 64;
    vector<unsigned long long> reach((size_t)K * W, 0);
    vector<vector<int>> byc(K);
    for (int v = 0; v < n; ++v) byc[comp[v]].push_back(v);
    for (int k = 0; k < K; ++k) {
      unsigned long long *rk = &reach[(size_t)k * W];
      rk[k / 64] |= 1ULL << (k % 64);
      for (int v : byc[k])
        for (auto &a : g[v])
          if (a.cap > 0 && comp[a.to] != k)
            for (int j = 0, c = comp[a.to]; j < W; ++j) rk[j] |= reach[(size_t)c * W + j];
    }
    auto to = [&](int x, int y) { return reach[(size_t)comp[x] * W + comp[y] / 64] >> (comp[y] % 64) & 1; };
    vector<int> ans;
    ans.reserve(qs.size());
    for (auto &[c, d] : qs) {
      const bool ac = to(s, c), ad = to(s, d), bc = to(c, t), bd = to(d, t);
      const bool same = (!bc && !bd) || (!ac && !ad);
      const bool diff = (!bc && !ad && !to(c, d)) || (!bd && !ac && !to(d, c));
      ans.push_back(same | diff << 1);
    }
    return ans;
  }
};
