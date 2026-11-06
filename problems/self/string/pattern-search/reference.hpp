#pragma once
// 参照実装。パターンの KMP の失敗関数を作り、テキストを 1 回走査する。
struct Solver {
  const string &t, &p;
  vector<int> pos;
  Solver(const string &t, const string &p) : t(t), p(p) {}
  void run() {
    const int n = (int)t.size(), m = (int)p.size();
    vector<int> pi(m, 0);
    for (int i = 1; i < m; ++i) {
      int k = pi[i - 1];
      while (k > 0 && p[i] != p[k]) k = pi[k - 1];
      if (p[i] == p[k]) ++k;
      pi[i] = k;
    }
    pos.clear();
    for (int i = 0, k = 0; i < n; ++i) {
      while (k > 0 && t[i] != p[k]) k = pi[k - 1];
      if (t[i] == p[k]) ++k;
      if (k == m) pos.push_back(i - m + 1), k = pi[k - 1];
    }
  }
  const vector<int> &answer() const { return pos; }
};
