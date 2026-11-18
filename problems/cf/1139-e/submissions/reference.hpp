#pragma once
// 期待出力を作る参照実装。_shared/kuhn の核とも今の Library とも別に書いた、よく知られた解き方 (2026-11-18)。日を逆に
// たどって学生を足し、力 mex の値から再帰の深さ優先で増加路を探す (探すたびに訪ねた部の印を消す)。小さい入力では
// brute.hpp (割り当てを全部試す) と、gen.py の seed 1000 から 2999 で突き合わせてある。
#include <vector>

struct Solver {
  int n, m;
  const vector<int> &p, &c, &k;
  vector<vector<int>> adj;  // 力 → 部
  vector<int> mate, vis;    // 部の相手の力
  int stamp = 0;
  Solver(int n, int m, const vector<int> &p, const vector<int> &c, const vector<int> &k) : n(n), m(m), p(p), c(c), k(k) {}
  bool dfs(int v) {
    for (int x : adj[v])
      if (vis[x] != stamp) {
        vis[x] = stamp;
        if (mate[x] < 0 || dfs(mate[x])) return mate[x] = v, true;
      }
    return false;
  }
  vector<int> run() {
    const int d = k.size();
    adj.assign(5000, {}), mate.assign(m, -1), vis.assign(m, 0);
    vector<char> leave(n, 0);
    for (int x : k) leave[x] = 1;
    for (int i = 0; i < n; ++i)
      if (!leave[i]) adj[p[i]].push_back(c[i]);
    vector<int> ans(d);
    int mex = 0;
    for (int day = d - 1; day >= 0; --day) {
      while (mex < 5000 && (++stamp, dfs(mex))) ++mex;
      ans[day] = mex;
      adj[p[k[day]]].push_back(c[k[day]]);
    }
    return ans;
  }
};
