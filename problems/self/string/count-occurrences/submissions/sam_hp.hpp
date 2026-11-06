#pragma once
// sam の表 (遷移の欄、suffix link、長さ、数) を huge page に置いたもの (_shared/string/_hugealloc.hpp)。104 MB の遷移の表を
// 飛び飛びにたどるので、4 KB のページでは TLB の取り違えとページを初めて触る手間が大きい。中身は sam と同じ接尾辞オートマトンで、
// 状態ごとに 26 個の遷移の欄 (0 は遷移なし)、suffix link、最長の長さを持ち、出現回数は長さの降順に suffix link の先へ足して求める。
#include "pj.hpp"
#include "_shared/string/_hugealloc.hpp"

struct Solver {
  string_hp::Vec<int> nxt, link, len, cnt;
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
