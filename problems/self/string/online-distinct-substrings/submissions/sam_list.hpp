#pragma once
// 遷移を辺の連結リストで持つ接尾辞オートマトン。状態は suffix link、最長の長さ、最初の辺 (12 byte) で、辺は文字、行き先、次の辺の
// 12 byte。遷移を探すたびにリストをたどり、新しい辺はリストの先頭に足す。複製の状態は元の状態の辺をすべて写す。辺は 3n 本以下。
#include "pj.hpp"

struct Solver {
  struct St {
    int link, len, head;
  };
  struct Edge {
    int to, next, c;
  };
  vector<St> st;
  vector<Edge> ed;
  int last = 0;
  u64 total = 0;
  Solver() : st(1, St{-1, 0, -1}) {}
  int get(int v, int c) const {
    for (int e = st[v].head; e >= 0; e = ed[e].next)
      if (ed[e].c == c) return ed[e].to;
    return 0;
  }
  void set(int v, int c, int to) {
    for (int e = st[v].head; e >= 0; e = ed[e].next)
      if (ed[e].c == c) {
        ed[e].to = to;
        return;
      }
    ed.push_back(Edge{to, st[v].head, c});
    st[v].head = (int)ed.size() - 1;
  }
  u64 push(char ch) {
    const int c = ch - 'a';
    const int cur = (int)st.size();
    st.push_back(St{0, st[last].len + 1, -1});
    int p = last;
    while (p != -1 && !get(p, c)) set(p, c, cur), p = st[p].link;
    if (p != -1) {
      const int q = get(p, c);
      if (st[p].len + 1 == st[q].len) {
        st[cur].link = q;
      } else {
        const int cl = (int)st.size();
        st.push_back(St{st[q].link, st[p].len + 1, -1});
        for (int e = st[q].head; e >= 0; e = ed[e].next) {
          const Edge x = ed[e];
          ed.push_back(Edge{x.to, st[cl].head, x.c});
          st[cl].head = (int)ed.size() - 1;
        }
        while (p != -1 && get(p, c) == q) set(p, c, cl), p = st[p].link;
        st[q].link = st[cur].link = cl;
      }
    }
    last = cur;
    total += st[cur].len - st[st[cur].link].len;
    return total;
  }
};
