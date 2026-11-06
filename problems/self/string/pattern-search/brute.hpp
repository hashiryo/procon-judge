#pragma once
// 愚直解。テキストのすべての位置でパターンと 1 文字ずつ比べる。小さい入力 (gen.py の seed 1000 以上) でだけ使う。
struct Solver {
  const string &t, &p;
  vector<int> pos;
  Solver(const string &t, const string &p) : t(t), p(p) {}
  void run() {
    pos.clear();
    for (size_t i = 0; i + p.size() <= t.size(); ++i)
      if (t.compare(i, p.size(), p) == 0) pos.push_back((int)i);
  }
  const vector<int> &answer() const { return pos; }
};
