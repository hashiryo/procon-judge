#pragma once
// 愚直解。頂点の部分集合ごとの最大マッチングの大きさを、番号の最も小さい頂点を空けるか隣と組ませるかで求める
// (2^n n)。小さい入力 (gen.py の seed 1000 以上、頂点 20 個まで) でだけ使う。
struct Solver {
  int n;
  const vector<array<int, 2>> &e;
  vector<int> ids;

  Solver(int n, const vector<array<int, 2>> &edges) : n(n), e(edges) {}

  void run() {
    vector<unsigned> nb(n);
    vector<vector<int>> eid(n, vector<int>(n, -1));
    for (int i = 0; i < (int)e.size(); ++i) {
      auto [u, v] = e[i];
      nb[u] |= 1u << v, nb[v] |= 1u << u, eid[u][v] = eid[v][u] = i;
    }
    vector<signed char> dp(1u << n);
    for (unsigned s = 1; s < (1u << n); ++s) {
      const int v = __builtin_ctz(s);
      const unsigned r = s & ~(1u << v);
      int best = dp[r];
      for (unsigned c = nb[v] & r; c; c &= c - 1) best = max(best, 1 + dp[r & ~(1u << __builtin_ctz(c))]);
      dp[s] = (signed char)best;
    }
    // 復元。最も小さい頂点から、dp を保つ選び方をたどる。
    for (unsigned s = (1u << n) - 1; s;) {
      const int v = __builtin_ctz(s);
      const unsigned r = s & ~(1u << v);
      if (dp[s] == dp[r]) {
        s = r;
        continue;
      }
      for (unsigned c = nb[v] & r; c; c &= c - 1) {
        const int u = __builtin_ctz(c);
        if (dp[s] == 1 + dp[r & ~(1u << u)]) {
          ids.push_back(eid[v][u]), s = r & ~(1u << u);
          break;
        }
      }
    }
  }

  vector<int> answer() const { return ids; }
};
