#pragma once
// 広義の接尾辞オートマトンを、状態ごとに 26 個の遷移の欄 (0 は遷移なし) を持つ形で作るもの。作り方は gsam_art と同じ。
#include "pj.hpp"
#include <cstring>

struct Solver {
  struct St {
    int next[26], link, len;
  };
  const vector<string> &ss;
  vector<St> st;
  u64 total = 0;
  explicit Solver(const vector<string> &ss) : ss(ss) {}
  int make(int len, int link) {
    st.emplace_back();
    St &v = st.back();
    memset(v.next, 0, sizeof(v.next));
    v.link = link, v.len = len;
    return (int)st.size() - 1;
  }
  int split(int p, int c, int q, int l) {
    const int cl = make(l, st[q].link);
    memcpy(st[cl].next, st[q].next, sizeof(st[q].next));
    while (p != -1 && st[p].next[c] == q) st[p].next[c] = cl, p = st[p].link;
    st[q].link = cl;
    return cl;
  }
  int extend(int last, int c) {
    if (const int q = st[last].next[c]) return st[last].len + 1 == st[q].len ? q : split(last, c, q, st[last].len + 1);
    const int cur = make(st[last].len + 1, 0);
    int p = last;
    while (p != -1 && !st[p].next[c]) st[p].next[c] = cur, p = st[p].link;
    if (p != -1) {
      const int q = st[p].next[c];
      st[cur].link = st[p].len + 1 == st[q].len ? q : split(p, c, q, st[p].len + 1);
    }
    total += st[cur].len - st[st[cur].link].len;
    return cur;
  }
  void run() {
    st.clear(), total = 0;
    make(0, -1);
    for (const string &s : ss) {
      int last = 0;
      for (unsigned char ch : s) last = extend(last, ch - 'a');
    }
  }
  u64 answer() const { return total; }
};
