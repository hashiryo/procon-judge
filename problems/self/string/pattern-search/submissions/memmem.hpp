#pragma once
// C ライブラリの memmem を、見つけた位置の次から呼び直す。glibc の memmem は短いパターンを 2 文字ずつのハッシュの表で、長い
// パターンを 2 方向の照合 (two-way、Crochemore と Perrin) で探すので、1 回の呼び出しは線形。ただし呼ぶたびにパターンの前処理を
// し直すので、出現が密なテキストでは O(n m) かかる。
#include "pj.hpp"
#include <cstring>

struct Solver {
  const string &t, &p;
  vector<int> pos;
  Solver(const string &t, const string &p) : t(t), p(p) {}
  void run() {
    const char *base = t.data(), *end = base + t.size();
    const size_t m = p.size();
    pos.clear();
    for (const char *cur = base; size_t(end - cur) >= m;) {
      const void *r = memmem(cur, size_t(end - cur), p.data(), m);
      if (!r) break;
      const char *q = (const char *)r;
      pos.push_back(int(q - base));
      cur = q + 1;
    }
  }
  const vector<int> &answer() const { return pos; }
};
