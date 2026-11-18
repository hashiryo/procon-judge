#pragma once
// 期待出力を作る参照実装。_shared/kuhn の核とは別に書いた、よく知られた解き方 (2026-11-18)。候補の値 a_i × k (k ≤ n) を
// 小さい順に並べ、値ごとに、空いた漁師が隣にいればそれと組み、いなければ再帰の深さ優先で増加路を探す。訪ねた漁師の印は、
// 増加路が見つかったときだけ消す (見つからなかった探索で訪ねた漁師からは、空いた漁師へ届かない)。小さい入力では brute.hpp
// (順番を全部試して、問題文のとおりに言う値を求める) と、gen.py の seed 1000 から 2999 で突き合わせてある。
#include <algorithm>
#include <vector>

struct Solver {
  int n;
  const vector<long long> &a;
  vector<vector<int>> adj;  // 値の番号 → 漁師
  vector<int> mate_f, vis;
  int stamp = 1;
  Solver(int n, const vector<long long> &a) : n(n), a(a) {}
  bool dfs(int v) {
    for (int f : adj[v])
      if (mate_f[f] < 0) return mate_f[f] = v, true;
    for (int f : adj[v])
      if (vis[f] != stamp) {
        vis[f] = stamp;
        if (dfs(mate_f[f])) return mate_f[f] = v, true;
      }
    return false;
  }
  long long run() {
    vector<pair<long long, int>> c;
    for (int i = 0; i < n; ++i)
      for (long long k = 1; k <= n; ++k) c.push_back({a[i] * k, i});
    sort(c.begin(), c.end());
    vector<long long> val;
    for (auto &[v, i] : c) {
      if (val.empty() || val.back() != v) val.push_back(v), adj.emplace_back();
      adj.back().push_back(i);
    }
    mate_f.assign(n, -1), vis.assign(n, 0);
    long long sum = 0;
    int got = 0;
    for (int v = 0; v < (int)val.size() && got < n; ++v)
      if (dfs(v)) sum += val[v], ++got, ++stamp;
    return sum;
  }
};
