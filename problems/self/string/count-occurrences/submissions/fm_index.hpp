#pragma once
// FM-index。NeoLibrary の接尾辞配列 (neo/string/suffix_array.hpp) から、末尾に最小の番兵を足した文字列の BWT を作り、文字ごとに
// BWT の位置の bit の列と、64 個ごとのその文字の累積の数を 16 byte の組で持つ。count はパターンを後ろから読み、範囲 [lo, hi) を
// C[c] + Occ(c, lo) と C[c] + Occ(c, hi) に絞る (Occ は表を 1 回読んで popcount)。文字は使われているものだけに番号を振り直し、
// 表は文字ごとに並べる (同じ文字の lo と hi の読み込みが近くに来る)。テキストの長さ 5 * 10^5、26 種類で 3.3 MB。
#include "pj.hpp"
#include "neo/string/suffix_array.hpp"

struct Solver {
  struct Ent {
    u64 bits;
    u32 cnt, pad;
  };
  int n, sigma = 0, words;
  unsigned char code[256];
  vector<int> C;
  vector<Ent> occ;
  explicit Solver(const string &s) : n((int)s.size()) {
    const unsigned char *t = (const unsigned char *)s.data();
    int freq[256] = {};
    for (int i = 0; i < n; ++i) ++freq[t[i]];
    memset(code, 255, sizeof(code));
    C.clear();
    for (int c = 0, acc = 1; c < 256; ++c)
      if (freq[c]) code[c] = (unsigned char)sigma++, C.push_back(acc), acc += freq[c];
    const vector<int> sa = suffix_array(s);
    words = (n + 1) / 64 + 1;
    occ.assign((size_t)sigma * words, Ent{0, 0, 0});
    // BWT の位置 0 は番兵の接尾辞で、その前の文字は s[n - 1]。位置 r (1 <= r <= n) は sa[r - 1] の前の文字 (sa[r - 1] = 0 なら番兵)。
    auto put = [&](int r, int c) { occ[(size_t)c * words + (r >> 6)].bits |= (u64)1 << (r & 63); };
    put(0, code[t[n - 1]]);
    for (int r = 1; r <= n; ++r)
      if (sa[r - 1] > 0) put(r, code[t[sa[r - 1] - 1]]);
    for (int c = 0; c < sigma; ++c) {
      Ent *e = occ.data() + (size_t)c * words;
      u32 acc = 0;
      for (int w = 0; w < words; ++w) e[w].cnt = acc, acc += (u32)__builtin_popcountll(e[w].bits);
    }
  }
  int rank(int c, int i) const {
    const Ent &e = occ[(size_t)c * words + (i >> 6)];
    return (int)e.cnt + __builtin_popcountll(e.bits & (((u64)1 << (i & 63)) - 1));
  }
  int count(const string &p) {
    int lo = 0, hi = n + 1;
    for (int j = (int)p.size(); j--;) {
      const int c = code[(unsigned char)p[j]];
      if (c == 255) return 0;
      lo = C[c] + rank(c, lo), hi = C[c] + rank(c, hi);
      if (lo >= hi) return 0;
    }
    return hi - lo;
  }
};
