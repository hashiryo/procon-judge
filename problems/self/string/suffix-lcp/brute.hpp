#pragma once
// 愚直解。2 つの接尾辞を先頭から 1 文字ずつ比べる。小さい入力 (gen.py の seed 1000 以上) でだけ使う。
struct Solver {
  const string &s;
  explicit Solver(const string &s) : s(s) {}
  int lcp(int i, int j) {
    const int n = (int)s.size();
    int h = 0;
    while (i + h < n && j + h < n && s[i + h] == s[j + h]) ++h;
    return h;
  }
};
