#pragma once
// 接尾辞オートマトン。状態ごとに 26 個の遷移の欄 (0 は遷移なし、根は状態 0 で遷移の先にならない)、suffix link、最長の長さを持ち、
// テキストを 1 文字ずつ足して作る。出現回数 (endpos の大きさ) は、複製でない状態に 1 を置いてから、長さの降順 (数え上げの
// ソート) に suffix link の先へ足す。count はパターンの遷移をたどって、たどり着いた状態の数を返す。状態は 2n 個以下で、
// 遷移の表は n = 5 * 10^5 で 104 MB。
#include "pj.hpp"

struct Solver {
  vector<int> nxt, link, len, cnt;
  explicit Solver(const string &s) {
    const int n = (int)s.size(), cap = 2 * n + 1;
    nxt.assign((size_t)cap * 26, 0), link.assign(cap, -1), len.assign(cap, 0), cnt.assign(cap, 0);
    int sz = 1, last = 0;
    for (unsigned char ch : s) {
      const int c = ch - 'a', cur = sz++;
      len[cur] = len[last] + 1, cnt[cur] = 1;
      int p = last;
      while (p != -1 && !nxt[p * 26 + c]) nxt[p * 26 + c] = cur, p = link[p];
      if (p == -1) link[cur] = 0;
      else {
        const int q = nxt[p * 26 + c];
        if (len[p] + 1 == len[q]) link[cur] = q;
        else {
          const int cl = sz++;
          len[cl] = len[p] + 1, link[cl] = link[q];
          copy_n(nxt.begin() + (size_t)q * 26, 26, nxt.begin() + (size_t)cl * 26);
          while (p != -1 && nxt[p * 26 + c] == q) nxt[p * 26 + c] = cl, p = link[p];
          link[q] = link[cur] = cl;
        }
      }
      last = cur;
    }
    vector<int> bucket(n + 2, 0), order(sz);
    for (int v = 0; v < sz; ++v) ++bucket[len[v]];
    for (int i = 1; i <= n; ++i) bucket[i] += bucket[i - 1];
    for (int v = sz; v--;) order[--bucket[len[v]]] = v;
    for (int i = sz; --i > 0;) cnt[link[order[i]]] += cnt[order[i]];
  }
  int count(const string &p) {
    int v = 0;
    for (unsigned char ch : p)
      if (!(v = nxt[v * 26 + (ch - 'a')])) return 0;
    return cnt[v];
  }
};
