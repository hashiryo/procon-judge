#pragma once
// 参照実装。KMP の失敗関数の素直な形。pi[i - 1] から border をたどり、次の文字が s[i] と等しいものを探す。
struct Solver {
  const string &s;
  vector<int> pi;
  explicit Solver(const string &s) : s(s) {}
  void run() {
    const int n = (int)s.size();
    pi.assign(n, 0);
    for (int i = 1; i < n; ++i) {
      int k = pi[i - 1];
      while (k > 0 && s[i] != s[k]) k = pi[k - 1];
      if (s[i] == s[k]) ++k;
      pi[i] = k;
    }
  }
  const vector<int> &answer() const { return pi; }
};
