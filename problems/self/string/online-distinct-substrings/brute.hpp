#pragma once
// 愚直解。今の文字列のすべての部分文字列を std::set に入れ、文字を足すたびに新しい接尾辞をすべて入れる。小さい入力 (gen.py の
// seed 1000 以上) でだけ使う。
#include <set>

struct Solver {
  string s;
  set<string> st;
  u64 push(char c) {
    s += c;
    for (size_t i = 0; i < s.size(); ++i) st.insert(s.substr(i));
    return st.size();
  }
};
