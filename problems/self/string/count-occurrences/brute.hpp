#pragma once
// 愚直解。テキストのすべての位置でパターンと比べる。小さい入力 (gen.py の seed 1000 以上) でだけ使う。
struct Solver {
  const string &s;
  explicit Solver(const string &s) : s(s) {}
  int count(const string &p) {
    int c = 0;
    for (size_t i = 0; i + p.size() <= s.size(); ++i) c += s.compare(i, p.size(), p) == 0;
    return c;
  }
};
