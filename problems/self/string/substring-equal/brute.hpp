#pragma once
// 愚直解。2 つの部分文字列を 1 文字ずつ比べる。小さい入力 (gen.py の seed 1000 以上) でだけ使う。
struct Solver {
  const string &s;
  explicit Solver(const string &s) : s(s) {}
  bool equal(int a, int b, int len) {
    for (int k = 0; k < len; ++k)
      if (s[a + k] != s[b + k]) return false;
    return true;
  }
};
