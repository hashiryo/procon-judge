#pragma once
// prefix doubling の並べ直しを、2 つの順位の組の LSD の基数ソートで行う版。2 つ目の鍵 (rk[i + 2^k]) の順は前の回の接尾辞配列から
// 作れる (i + 2^k が n 以上の添字を先に、そのあと前の回の順で sa[j] - 2^k を並べる) ので、数え上げは 1 つ目の鍵の 1 回で済む。
// 組が変わるところで新しい順位を振り、順位がすべて違ったら止める。1 回が O(n) なので、全体は O(n log n)。
#include <algorithm>
#include "pj.hpp"

struct Solver {
  string s;
  vector<int> sa;

  explicit Solver(const string &s) : s(s) {}

  void run() {
    const int n = int(s.size());
    sa.resize(n);
    if (!n) return;
    vector<int> rk(n), old(n), y(n), cnt(std::max(256, n));
    int m = 256;
    for (int i = 0; i < n; ++i) ++cnt[rk[i] = (unsigned char)s[i]];
    for (int c = 1; c < m; ++c) cnt[c] += cnt[c - 1];
    for (int i = n; i--;) sa[--cnt[rk[i]]] = i;
    for (int k = 1;; k <<= 1) {
      // 2 つ目の鍵の順。i + k >= n の添字は 2 つ目の鍵が空 (最も小さい) なので先に置く。
      int p = 0;
      for (int i = std::max(0, n - k); i < n; ++i) y[p++] = i;
      for (int j = 0; j < n; ++j)
        if (sa[j] >= k) y[p++] = sa[j] - k;
      // 1 つ目の鍵で安定に数え上げる。
      std::fill(cnt.begin(), cnt.begin() + m, 0);
      for (int i = 0; i < n; ++i) ++cnt[rk[i]];
      for (int c = 1; c < m; ++c) cnt[c] += cnt[c - 1];
      for (int j = n; j--;) sa[--cnt[rk[y[j]]]] = y[j];
      // 組が変わるところで新しい順位を振る。
      std::swap(old, rk);
      rk[sa[0]] = 0;
      p = 1;
      for (int j = 1; j < n; ++j) {
        const int a = sa[j - 1], b = sa[j];
        const bool same = old[a] == old[b] && (a + k < n ? old[a + k] : -1) == (b + k < n ? old[b + k] : -1);
        rk[b] = same ? p - 1 : p++;
      }
      if (p == n) break;  // 順位がすべて違う
      m = p;
    }
  }

  const vector<int> &answer() const { return sa; }
};
