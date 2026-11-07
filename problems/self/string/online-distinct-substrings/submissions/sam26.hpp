#pragma once
// 接尾辞オートマトン。状態ごとに 26 個の遷移の欄 (0 は遷移なし、根は状態 0 で遷移の先にならない)、suffix link、最長の長さを
// 1 つの 112 byte の組に持ち、vector で伸ばす。文字を足すと、異なる部分文字列の数は len[cur] - len[link[cur]] だけ増える。
#include "pj.hpp"
#include <cstring>

struct Solver {
  struct St {
    int next[26], link, len;
  };
  vector<St> st;
  int last = 0;
  u64 total = 0;
  Solver() : st(1) {
    memset(st[0].next, 0, sizeof(st[0].next));
    st[0].link = -1, st[0].len = 0;
  }
  int make(int len, int link) {
    st.emplace_back();
    St &v = st.back();
    memset(v.next, 0, sizeof(v.next));
    v.link = link, v.len = len;
    return (int)st.size() - 1;
  }
  u64 push(char ch) {
    const int c = ch - 'a';
    const int cur = make(st[last].len + 1, 0);
    int p = last;
    while (p != -1 && !st[p].next[c]) st[p].next[c] = cur, p = st[p].link;
    if (p != -1) {
      const int q = st[p].next[c];
      if (st[p].len + 1 == st[q].len) {
        st[cur].link = q;
      } else {
        const int cl = make(st[p].len + 1, st[q].link);
        memcpy(st[cl].next, st[q].next, sizeof(st[q].next));
        while (p != -1 && st[p].next[c] == q) st[p].next[c] = cl, p = st[p].link;
        st[q].link = st[cur].link = cl;
      }
    }
    last = cur;
    total += st[cur].len - st[st[cur].link].len;
    return total;
  }
};
