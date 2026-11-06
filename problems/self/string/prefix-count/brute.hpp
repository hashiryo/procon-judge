#pragma once
// 愚直解。要素を std::multiset に持ち、count_prefix ではすべての要素の先頭を比べる。小さい入力 (gen.py の seed 1000 以上) でだけ使う。
#include <set>

struct Solver {
  multiset<string> st;
  void insert(const string &s) { st.insert(s); }
  void erase(const string &s) { st.erase(st.find(s)); }
  int count_prefix(const string &p) {
    int c = 0;
    for (const string &s : st) c += s.compare(0, p.size(), p) == 0;
    return c;
  }
};
