#pragma once
// 状態 k が 64 未満の間は、KMP のオートマトンの遷移の表 (64 行、文字は使われているものに番号を振り直して幅 32) を 1 回引いて
// 次の状態を求め、64 以上なら分岐のある KMP に戻る。行 i (i < 64) は行 pi[i - 1] を写して s[i] の欄を i + 1 にしたもので、
// 表は 8 KB で L1 に載る。一様な文字列では状態がほとんど 64 未満なので、border をたどるループの分岐の外れが無くなり、1 文字に
// 表の読み込み 1 回の鎖になる。周期的な列では状態が大きく、一致が続く分岐は外れないので、KMP のまま速い。
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
    unsigned char code[256] = {};
    {
      bool seen[256] = {};
      for (int i = 0; i < n; ++i) seen[t[i]] = true;
      int sigma = 0;
      for (int c = 0; c < 256; ++c)
        if (seen[c]) code[c] = (unsigned char)(sigma++ & 31);
    }
    constexpr int R = 64, W = 32;
    alignas(64) static int d[R * W];
    memset(d, 0, sizeof(int) * W);
    d[code[t[0]]] = 1;
    int *p = pi.data();
    p[0] = 0;
    int k = 0;
    for (int i = 1; i < n; ++i) {
      const int c = code[t[i]];
      if (i < R) {  // 行 i を作る (pi[i - 1] = k はもう決まっている)
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
