#pragma once
// 各頂点に 26 個の子の欄を持つトライ。子の欄は 1 本の配列に頂点ごとに 26 個ずつ並べ (0 は子なし、根は頂点 0)、頂点を通る要素の数は
// 別の配列に持つ。
#include "pj.hpp"

struct Solver {
  vector<int> ch, cnt;
  Solver() : ch(26, 0), cnt(1, 0) {}
  void add(const string &s, int d) {
    int v = 0;
    cnt[0] += d;
    for (unsigned char c : s) {
      int w = ch[v * 26 + (c - 'a')];
      if (!w) {
        w = (int)cnt.size();
        ch[v * 26 + (c - 'a')] = w;
        ch.resize(ch.size() + 26, 0);
        cnt.push_back(0);
      }
      cnt[w] += d;
      v = w;
    }
  }
  void insert(const string &s) { add(s, 1); }
  void erase(const string &s) { add(s, -1); }
  int count_prefix(const string &p) {
    int v = 0;
    for (unsigned char c : p)
      if (!(v = ch[v * 26 + (c - 'a')])) return 0;
    return cnt[v];
  }
};
