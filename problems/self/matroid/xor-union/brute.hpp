#pragma once
// 愚直解。ベクトルごとに組の番号か「使わない」を全部試し (残りを全部足しても今の最大に届かない枝は刈る)、割り当てた
// ベクトルの数が最大のものを返す。小さい入力 (gen.py の seed 1000 以上、ベクトル 10 本まで、組 3 つまで) でだけ使う。
struct Solver {
  const vector<u64> &v;
  int k;
  vector<int> cur, best;
  int best_cnt = -1;

  Solver(const vector<u64> &vec, int k) : v(vec), k(k) {}

  // b[j] は組 j の掃き出した行
  void dfs(int i, int cnt, vector<array<u64, 64>> &b) {
    const int m = v.size();
    if (cnt + (m - i) <= best_cnt) return;
    if (i == m) {
      best_cnt = cnt, best = cur;
      return;
    }
    for (int j = 0; j < k; ++j) {
      u64 x = v[i];
      while (x && b[j][63 - __builtin_clzll(x)]) x ^= b[j][63 - __builtin_clzll(x)];
      if (!x) continue;
      b[j][63 - __builtin_clzll(x)] = x;
      cur[i] = j;
      dfs(i + 1, cnt + 1, b);
      b[j][63 - __builtin_clzll(x)] = 0;
    }
    cur[i] = -1;
    dfs(i + 1, cnt, b);
  }

  void run() {
    cur.assign(v.size(), -1);
    vector<array<u64, 64>> b(k);
    for (auto &r : b) r.fill(0);
    dfs(0, 0, b);
  }

  vector<int> answer() const { return best; }
};
