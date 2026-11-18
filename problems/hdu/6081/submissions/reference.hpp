#pragma once
// 期待出力を作る参照実装。_shared/gmincut とは別に書いた Nagamochi-Ono-Ibaraki の方法 (2026-11-18)。上界 λ を重み付きの
// 次数の最小で始め、maximum adjacency の順に頂点をなめる。なめた頂点 v の隣の未訪問の w について、なめた頂点から w への
// 重みの和 r(w) が λ を越えたら v と w をまとめる (r(w) は v と w の局所連結度の下界)。なめた頂点の集合のカットでも λ を
// 下げる。まとめた組が無ければ最後の 2 頂点をまとめる。まとめた頂点を縮めて辺を並べ直して足し、頂点が 1 つになるまで
// 繰り返す。小さい入力では brute.hpp (頂点集合を全部試す) と、gen.py の seed 1000 から 2999 で突き合わせてある。
#include <algorithm>
#include <climits>
#include <numeric>
#include <queue>
#include <vector>

struct Solver {
  int n;
  const vector<array<i64, 3>> &e;
  Solver(int n, const vector<array<i64, 3>> &e) : n(n), e(e) {}
  static void normalize(vector<array<i64, 3>> &E) {
    sort(E.begin(), E.end(), [](auto &a, auto &b) { return a[0] != b[0] ? a[0] < b[0] : a[1] < b[1]; });
    size_t k = 0;
    for (size_t i = 0; i < E.size(); ++i)
      if (k > 0 && E[k - 1][0] == E[i][0] && E[k - 1][1] == E[i][1]) E[k - 1][2] += E[i][2];
      else E[k++] = E[i];
    E.resize(k);
  }
  i64 run() {
    int k = n;
    vector<array<i64, 3>> E;
    for (auto &[u, v, w] : e)
      if (u != v) E.push_back({min(u, v), max(u, v), w});
    normalize(E);
    i64 lam = LLONG_MAX;
    vector<i64> deg;
    auto degs = [&]() {
      deg.assign(k, 0);
      for (auto &[u, v, w] : E) deg[u] += w, deg[v] += w;
      for (i64 d : deg) lam = min(lam, d);
    };
    degs();
    while (k > 1 && lam > 0) {
      vector<vector<pair<int, i64>>> adj(k);
      for (auto &[u, v, w] : E) adj[u].push_back({(int)v, w}), adj[v].push_back({(int)u, w});
      vector<i64> r(k, 0);
      vector<char> done(k, 0);
      vector<int> uf(k);
      iota(uf.begin(), uf.end(), 0);
      auto find = [&](int x) {
        while (uf[x] != x) x = uf[x] = uf[uf[x]];
        return x;
      };
      bool any = false;
      auto unite = [&](int a, int b) {
        a = find(a), b = find(b);
        if (a != b) uf[a] = b, any = true;
      };
      priority_queue<pair<i64, int>> pq;
      for (int v = 0; v < k; ++v) pq.push({0, v});
      i64 cut = 0;
      int cnt = 0, last = -1, prev = -1;
      while (!pq.empty()) {
        auto [key, v] = pq.top();
        pq.pop();
        if (done[v] || key != r[v]) continue;
        done[v] = 1, ++cnt, prev = last, last = v;
        cut += deg[v] - 2 * r[v];
        if (cnt < k) lam = min(lam, cut);
        for (auto [w, c] : adj[v])
          if (!done[w]) {
            if (r[w] < lam && r[w] + c >= lam) unite(v, w);
            r[w] += c, pq.push({r[w], w});
          }
      }
      if (lam == 0) break;
      if (!any) unite(last, prev);
      vector<int> id(k, -1);
      int nk = 0;
      for (int v = 0; v < k; ++v)
        if (find(v) == v) id[v] = nk++;
      vector<array<i64, 3>> F;
      for (auto &[u, v, w] : E) {
        const i64 a = id[find(u)], b = id[find(v)];
        if (a != b) F.push_back({min(a, b), max(a, b), w});
      }
      normalize(F);
      E.swap(F), k = nk;
      if (k > 1) degs();
    }
    return lam == LLONG_MAX ? 0 : lam;
  }
};
