#pragma once
// 接尾辞配列の上で p で始まる接尾辞の範囲を二分探索する形を 2 つ直したもの。1 つ目は先頭 2 文字の表で、接尾辞配列のうち
// 先頭 2 byte が同じ範囲の始まりを 65536 + 1 個の表に持ち、パターンの先頭 2 文字の範囲から探す (1 文字のパターンは表だけで
// 答える)。2 つ目は比べ方で、テキストとパターンの末尾に 0 を足した写しを 8 byte ずつ排他的論理和で比べ、memcmp を呼ばない
// (英小文字は 0 でないので、短い方の終わりで必ず食い違う)。
#include "pj.hpp"
#include <algorithm>
#include <cstring>
#include "neo/string/suffix_array.hpp"

struct Solver {
  int n;
  string t;
  vector<int> sa, head;
  string pb;
  explicit Solver(const string &s) : n((int)s.size()), t(s), sa(suffix_array(s)), head(65537, 0) {
    t.append(8, '\0');
    const unsigned char *u = (const unsigned char *)t.data();
    for (int i = 0; i < n; ++i) ++head[(u[i] << 8 | u[i + 1]) + 1];
    for (int k = 0; k < 65536; ++k) head[k + 1] += head[k];
  }
  // s[i + 2, ...) の先頭 m - 2 文字と、p の 3 文字目からを比べる。負なら接尾辞が小さい、0 なら p で始まる。
  int cmp(int i, int m) const {
    const char *x = t.data() + i, *y = pb.data();
    for (int j = 2; j < m; j += 8) {
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
    const unsigned char c0 = (unsigned char)p[0];
    if (m == 1) return head[(c0 + 1) << 8] - head[c0 << 8];
    const int key = c0 << 8 | (unsigned char)p[1];
    const int *lo = sa.data() + head[key], *hi = sa.data() + head[key + 1];
    if (m == 2 || lo == hi) return int(hi - lo);
    pb.assign(p);
    pb.append(8, '\0');
    const int *a = partition_point(lo, hi, [&](int i) { return cmp(i, m) < 0; });
    const int *b = partition_point(a, hi, [&](int i) { return cmp(i, m) == 0; });
    return int(b - a);
  }
};
