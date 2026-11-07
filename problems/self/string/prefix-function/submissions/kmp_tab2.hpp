#pragma once
// kmp_tab と kmp_br を、使われている文字の種類で使い分けるもの。4 種類以下なら、状態が 64 未満の間は遷移の表 (64 行、幅 4) を
// 引き、64 以上は分岐のある KMP に戻る (2 文字の一様な列では border をたどる分岐が半分ほど外れるため)。5 種類以上なら kmp_br。
// 文字の種類が多い一様な列では、t[i] = t[k] がほとんど成り立たず分岐が外れないので、表を引く鎖より分岐の方が速い。
#include "pj.hpp"
#include <cstring>

struct Solver {
  const string &s;
  vector<int> pi;
  explicit Solver(const string &s) : s(s) {}
  void run() {
    const int n = (int)s.size();
    pi.resize(n);
    const unsigned char *t = (const unsigned char *)s.data();
    int *p = pi.data();
    p[0] = 0;
    unsigned char code[256] = {};
    int sigma = 0;
    {
      bool seen[256] = {};
      for (int i = 0; i < n; ++i) seen[t[i]] = true;
      for (int c = 0; c < 256; ++c)
        if (seen[c]) code[c] = (unsigned char)(sigma++ & 3);
    }
    if (sigma > 4) {
      for (int i = 1, k = 0; i < n; ++i) {
        if (t[i] == t[k]) {
          ++k;
        } else if (k > 0) {
          do k = p[k - 1];
          while (k > 0 && t[i] != t[k]);
          if (t[i] == t[k]) ++k;
        }
        p[i] = k;
      }
      return;
    }
    constexpr int R = 64, W = 4;
    alignas(64) int d[R * W] = {};
    d[code[t[0]]] = 1;
    int k = 0;
    for (int i = 1; i < n; ++i) {
      const int c = code[t[i]];
      if (i < R) {
        memcpy(d + i * W, d + k * W, sizeof(int) * W);
        d[i * W + c] = i + 1;
      }
      if (k < R) {
        k = d[k * W + c];
      } else if (t[i] == t[k]) {
        ++k;
      } else {
        do k = p[k - 1];
        while (k >= R && t[i] != t[k]);
        k = k >= R ? k + 1 : d[k * W + c];
      }
      p[i] = k;
    }
  }
  const vector<int> &answer() const { return pi; }
};
