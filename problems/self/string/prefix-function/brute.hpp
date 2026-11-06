#pragma once
// 愚直解。定義どおり、s[0, i] の border の長さを長い方から 1 つずつ試す。小さい入力 (gen.py の seed 1000 以上) でだけ使う。
struct Solver {
  const string &s;
  vector<int> pi;
  explicit Solver(const string &s) : s(s) {}
  void run() {
    const int n = (int)s.size();
    pi.assign(n, 0);
    for (int i = 0; i < n; ++i)
      for (int k = i; k > 0; --k)
        if (s.compare(0, k, s, i + 1 - k, k) == 0) {
          pi[i] = k;
          break;
        }
  }
  const vector<int> &answer() const { return pi; }
};
