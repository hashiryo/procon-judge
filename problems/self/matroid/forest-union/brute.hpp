#pragma once
// 愚直解。辺ごとに森の番号か「使わない」を全部試し (残りを全部足しても今の最大に届かない枝は刈る)、割り当てた辺の数が
// 最大のものを返す。小さい入力 (gen.py の seed 1000 以上、辺 12 本まで、森 3 本まで) でだけ使う。
#include <numeric>

struct Solver {
  int n, k;
  const vector<array<int, 2>> &e;
  vector<int> cur, best;
  int best_cnt = -1;

  Solver(int n, const vector<array<int, 2>> &edges, int k) : n(n), k(k), e(edges) {}

  // lab[j][v] は森 j での v の連結成分の名前
  void dfs(int i, int cnt, vector<vector<int>> &lab) {
    const int m = e.size();
    if (cnt + (m - i) <= best_cnt) return;
    if (i == m) {
      best_cnt = cnt, best = cur;
      return;
    }
    for (int j = 0; j < k; ++j) {
      const int a = lab[j][e[i][0]], b = lab[j][e[i][1]];
      if (a == b) continue;
      const vector<int> save = lab[j];
      for (auto &x : lab[j])
        if (x == a) x = b;
      cur[i] = j;
      dfs(i + 1, cnt + 1, lab);
      lab[j] = save;
    }
    cur[i] = -1;
    dfs(i + 1, cnt, lab);
  }

  void run() {
    cur.assign(e.size(), -1);
    vector<vector<int>> lab(k, vector<int>(n));
    for (auto &l : lab) iota(l.begin(), l.end(), 0);
    dfs(0, 0, lab);
  }

  vector<int> answer() const { return best; }
};
