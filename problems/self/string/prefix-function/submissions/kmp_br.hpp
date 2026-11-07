#pragma once
// KMP の一致の判定を分岐で書いたもの。kmp の k += (t[i] == t[k]) を GCC は sete と add にするので、次の文字で読む t[k] の位置が
// 前の読み込みの結果を待つ鎖になり、x64 では構造のある列でも 1 文字に 3.4 ns ほどかかっていた。分岐にすると、外れない列では
// 投機で次の文字へ進める。食い違ったときだけ border をたどる。
#include "pj.hpp"

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
  }
  const vector<int> &answer() const { return pi; }
};
