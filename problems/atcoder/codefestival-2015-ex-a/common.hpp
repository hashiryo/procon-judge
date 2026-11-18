#pragma once
// lib と lib-neo が共有する、残余グラフの強連結成分から問いに答える部分。comp は成分を縮めた DAG のトポロジカル順の番号
// (残余の弧 u → v なら comp[u] <= comp[v])。成分ごとに届く成分の集合を bitset で持ち、後ろの成分から順に、行き先の成分の
// 集合を足して作る。答え方は submissions/reference.hpp の説明と同じ。
#include <vector>

inline vector<int> answer_pairs(int n, int K, const vector<int> &comp, const vector<array<int, 2>> &arcs,
                                const vector<array<int, 2>> &qs) {
  const int s = 0, t = n - 1, W = (K + 63) / 64;
  vector<unsigned long long> reach((size_t)K * W, 0);
  vector<vector<int>> out(K);
  for (auto &[u, v] : arcs)
    if (comp[u] != comp[v]) out[comp[u]].push_back(comp[v]);
  for (int k = K; k--;) {
    unsigned long long *rk = &reach[(size_t)k * W];
    rk[k / 64] |= 1ULL << (k % 64);
    for (int c : out[k])
      for (int j = 0; j < W; ++j) rk[j] |= reach[(size_t)c * W + j];
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
