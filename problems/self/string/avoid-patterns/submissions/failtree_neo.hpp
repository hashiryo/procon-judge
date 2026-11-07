#pragma once
// failtree の DP を、トライと suffix link を NeoLibrary の aho_corasick (neo/string/aho_corasick.hpp) で求めて行うもの。aho_corasick の
// 頂点の番号は文字列を入れた順なので、親ごとの子の列を作って幅優先の順 (同じ親の子が並ぶ順) に並べ直す。トライはパターンに当たった状態の先も持つので、死んだ状態の
// 子孫を落として詰め直す (死んだ状態そのものは引く側に要るので残す)。DP は failtree と同じ 3 回の走査。
#include "pj.hpp"
#include "neo/string/aho_corasick.hpp"

struct Solver {
  const vector<string> &ps;
  int sigma;
  Solver(const vector<string> &ps, int sigma) : ps(ps), sigma(sigma) {}
  u32 count(int n) {
    constexpr u64 MOD = 998244353;
    const AhoCorasick ac = aho_corasick(ps);
    const int K = (int)ac.par.size();
    // 親ごとの子の列 (CSR) を作り、根から幅優先にたどって番号 idx を振る。upto[d] は深さ d 以下の頂点の数。
    vector<int> off(K + 1, 0), kids(max(K - 1, 1)), idx(K), bfs(K);
    for (int v = 1; v < K; ++v) ++off[ac.par[v] + 1];
    for (int v = 0; v < K; ++v) off[v + 1] += off[v];
    {
      vector<int> pos(off.begin(), off.end() - 1);
      for (int v = 1; v < K; ++v) kids[pos[ac.par[v]]++] = v;
    }
    const int deg0 = off[1] - off[0];
    vector<int> upto;
    bfs[0] = 0, idx[0] = 0;
    for (int h = 0, t = 1, lvl_end = 1; h < K; ++h) {
      if (h == lvl_end) upto.push_back(lvl_end), lvl_end = t;
      const int v = bfs[h];
      for (int j = off[v]; j < off[v + 1]; ++j) idx[kids[j]] = t, bfs[t++] = kids[j];
    }
    upto.push_back(K);
    const int D = (int)upto.size() - 1;
    vector<int> par(K), link(K);
    vector<unsigned char> dead(K, 0);
    for (int v = 0; v < K; ++v) par[idx[v]] = idx[ac.par[v]], link[idx[v]] = idx[ac.link[v]];
    for (const int w : ac.word) dead[idx[w]] = 1;
    for (int i = 1; i < K; ++i) dead[i] |= dead[link[i]];  // link[i] < i なので先に決まっている
    // 親が残っていて死んでいない頂点だけを残し、幅優先の順のまま詰める。残した頂点の suffix link は残した頂点を指す。
    vector<int> nid(K, -1);
    int k = 1;
    nid[0] = 0;
    for (int i = 1; i < K; ++i)
      if (nid[par[i]] >= 0 && !dead[par[i]]) nid[i] = k++;
    vector<u64> mask(k);
    {
      vector<int> kept(D + 1, 0);  // 深さ d 以下の残した頂点の数
      for (int i = 0, d = 0, j = 0; i < K; ++i) {
        while (upto[d] <= i) ++d;  // i 番目の頂点の深さ
        if (nid[i] < 0) continue;
        par[j] = nid[par[i]], link[j] = nid[link[i]], mask[j] = dead[i] ? 0 : ~0ull;
        kept[d] = ++j;
      }
      for (int e = 1; e <= D; ++e) kept[e] = max(kept[e], kept[e - 1]);
      upto.swap(kept);
    }
    const int r1 = upto[min(1, D)];
    vector<u64> s(k, 0), nw(k, 0);
    s[0] = 1;
    for (int i = 1; i <= n; ++i) {
      const int rp = upto[min(i - 1, D)], rc = upto[min(i, D)];
      for (int x = rp - 1; x >= 1; --x) s[link[x]] += s[x];
      nw[0] = u64(sigma - deg0) * s[0];
      for (int v = 1; v < min(r1, rc); ++v) nw[v] = s[0];
      for (int v = r1; v < rc; ++v) {
        const u64 g = s[par[v]];
        nw[v] = g;
        nw[link[v]] -= g;
      }
      for (int v = 0; v < rc; ++v) s[v] = (nw[v] % MOD) & mask[v];
    }
    u64 ans = 0;
    for (int v = 0; v < k; ++v) ans += s[v];
    return u32(ans % MOD);
  }
};
