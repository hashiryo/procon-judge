#pragma once
// 子を「最初の子」と「次の兄弟」の連結リストで持つトライ。頂点ごとに文字、最初の子、次の兄弟、通る要素の数の 16 byte で、子を探す
// たびにリストをたどる。新しい子はリストの先頭に足す。
#include "pj.hpp"

struct Solver {
  struct Node {
    int first, next, cnt;
    unsigned char c;
  };
  vector<Node> nd;
  Solver() : nd(1, Node{0, 0, 0, 0}) {}
  int child(int v, unsigned char c) const {
    int w = nd[v].first;
    while (w && nd[w].c != c) w = nd[w].next;
    return w;
  }
  void add(const string &s, int d) {
    int v = 0;
    nd[0].cnt += d;
    for (unsigned char c : s) {
      int w = child(v, c);
      if (!w) {
        w = (int)nd.size();
        nd.push_back(Node{0, nd[v].first, 0, c});
        nd[v].first = w;
      }
      nd[w].cnt += d;
      v = w;
    }
  }
  void insert(const string &s) { add(s, 1); }
  void erase(const string &s) { add(s, -1); }
  int count_prefix(const string &p) {
    int v = 0;
    for (unsigned char c : p)
      if (!(v = child(v, c))) return 0;
    return nd[v].cnt;
  }
};
