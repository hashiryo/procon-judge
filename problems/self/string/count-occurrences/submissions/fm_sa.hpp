#pragma once
// FM-index と接尾辞配列の二分探索を、パターンの長さで使い分けるもの。長さ 32 以下は FM-index (BWT と、文字ごとの bit の列と
// 64 個ごとの累積) で後ろから絞り (1 文字に表の読み込み 2 回)、33 以上は接尾辞配列の二分探索 (先頭 2 文字の表で狭め、8 byte ずつ
// 比べる) で求める。長いパターンでは、二分探索の log n 回の比べは前から続けて読むだけなので、FM-index の 1 文字ごとの飛び飛びの
// 読み込みより速い。先頭 2 文字の表 (65536 + 1 個) は、テキストが 2^14 文字以上のときだけ作る (短いテキストを何度も作る使い方で、
// 256 KB を 0 で埋める手間が目立つため)。
#include "pj.hpp"
#include <algorithm>
#include <cstring>
#include "neo/string/suffix_array.hpp"

struct Solver {
  struct Ent {
    u64 bits;
    u32 cnt, pad;
  };
  int n, sigma = 0, words;
  unsigned char code[256];
  int C[257], freq[256] = {};
  string t, pb;
  vector<int> sa, head;
  vector<Ent> occ;
  explicit Solver(const string &s) : n((int)s.size()), t(s), sa(suffix_array(s)) {
    const unsigned char *u = (const unsigned char *)s.data();
    for (int i = 0; i < n; ++i) ++freq[u[i]];
    memset(code, 255, sizeof(code));
    for (int c = 0, acc = 1; c < 256; ++c)
      if (freq[c]) C[sigma] = acc, code[c] = (unsigned char)sigma++, acc += freq[c];
    words = (n + 1) / 64 + 1;
    occ.assign((size_t)sigma * words, Ent{0, 0, 0});
    auto put = [&](int r, int c) { occ[(size_t)c * words + (r >> 6)].bits |= (u64)1 << (r & 63); };
    put(0, code[u[n - 1]]);
    for (int r = 1; r <= n; ++r)
      if (sa[r - 1] > 0) put(r, code[u[sa[r - 1] - 1]]);
    for (int c = 0; c < sigma; ++c) {
      Ent *e = occ.data() + (size_t)c * words;
      u32 acc = 0;
      for (int w = 0; w < words; ++w) e[w].cnt = acc, acc += (u32)__builtin_popcountll(e[w].bits);
    }
    t.append(8, '\0');
    if (n >= (1 << 14)) {
      head.assign(65537, 0);
      const unsigned char *v = (const unsigned char *)t.data();
      for (int i = 0; i < n; ++i) ++head[(v[i] << 8 | v[i + 1]) + 1];
      for (int k = 0; k < 65536; ++k) head[k + 1] += head[k];
    }
  }
  int rank(int c, int i) const {
    const Ent &e = occ[(size_t)c * words + (i >> 6)];
    return (int)e.cnt + __builtin_popcountll(e.bits & (((u64)1 << (i & 63)) - 1));
  }
  int fm(const string &p) const {
    int j = (int)p.size() - 1, c = code[(unsigned char)p[j]];
    if (c == 255) return 0;
    int lo = C[c], hi = C[c] + freq[(unsigned char)p[j]];
    while (j--) {
      c = code[(unsigned char)p[j]];
      if (c == 255) return 0;
      lo = C[c] + rank(c, lo), hi = C[c] + rank(c, hi);
      if (lo >= hi) return 0;
    }
    return hi - lo;
  }
  // s[i, ...) の先頭 m 文字のうち from 文字目からと、p の from 文字目からを比べる。
  int cmp(int i, int from, int m) const {
    const char *x = t.data() + i, *y = pb.data();
    for (int j = from; j < m; j += 8) {
      u64 a, b;
      memcpy(&a, x + j, 8), memcpy(&b, y + j, 8);
      if (a != b) {
        const int d = j + (__builtin_ctzll(a ^ b) >> 3);
        if (d >= m) return 0;
        return (unsigned char)x[d] < (unsigned char)y[d] ? -1 : 1;
      }
    }
    return 0;
  }
  int count(const string &p) {
    const int m = (int)p.size();
    if (m <= 32) return fm(p);
    if (m > n) return 0;
    const int *lo = sa.data(), *hi = sa.data() + n, from = head.empty() ? 0 : 2;
    if (from) {
      const int key = (unsigned char)p[0] << 8 | (unsigned char)p[1];
      lo = sa.data() + head[key], hi = sa.data() + head[key + 1];
      if (lo == hi) return 0;
    }
    pb.assign(p);
    pb.append(8, '\0');
    const int *a = partition_point(lo, hi, [&](int i) { return cmp(i, from, m) < 0; });
    const int *b = partition_point(a, hi, [&](int i) { return cmp(i, from, m) == 0; });
    return int(b - a);
  }
};
