#pragma once
// lyndon_ll で、根を伸ばすときに文字を 8 バイトずつ比べる版。前後に 8 バイトの余白を付けた写しを作り、根 [i, j) の後ろへは i と j
// から、前へは i と j の手前の 8 バイトを読んで排他的論理和を取り、後ろへは下位から、前へは上位から 0 のバイトを数える。どちらも
// 8 未満で決まれば、それだけで run かを決める (一様な文字列の根はほぼすべてこの道)。8 バイトとも一致したときだけ、8 バイトずつの
// ループで伸ばす。ループで比べたバイトの数は lyndon_ll と同じく予算から引き、全体で 32n を超えたら LCE と LCS を LCP 配列と
// RangeMin で引く形に切り替える (最悪でも O(n))。手元の M2 の max_random_00 では、根を調べる段が 1.49 ms から 0.93 ms になった。
// run の列挙の実装を比べた記録は algo-notes の notes/runs.md にある。
#include <algorithm>
#include <array>
#include <bit>
#include <cstdint>
#include <cstring>
#include <string>
#include <utility>
#include <vector>
#include "pj.hpp"
#include "neo/string/suffix_array.hpp"
#include "_lyndon_runs.hpp"
#include "_lyndon_hybrid.hpp"

namespace lyndon_ll_w8 {
inline uint64_t load(const unsigned char *p) {
  uint64_t x;
  memcpy(&x, p, 8);
  return x;
}
// t[i + k] = t[j + k] が続く長さ。e 文字までは一致していると分かっているときに、lim 文字を上限として求める。
inline int forward(const unsigned char *t, int i, int j, int e, int lim) {
  for (; e < lim; e += 8)
    if (const uint64_t x = load(t + i + e) ^ load(t + j + e)) return min(e + (countr_zero(x) >> 3), lim);
  return lim;
}
// t[i - 1 - k] = t[j - 1 - k] が続く長さ。b 文字までは一致していると分かっているときに、lim 文字を上限とし、stop 文字以上と分かった
// ところで止める (止めたときの値は下限)。
inline int backward(const unsigned char *t, int i, int j, int b, int lim, int stop) {
  for (; b < lim && b < stop; b += 8)
    if (const uint64_t y = load(t + i - 8 - b) ^ load(t + j - 8 - b)) return min(b + (countl_zero(y) >> 3), lim);
  return min(b, lim);
}
}  // namespace lyndon_ll_w8

struct Solver {
  string s;
  vector<array<int, 3>> ans;

  explicit Solver(const string &s) : s(s) {}

  void run() {
    using namespace lyndon_ll_w8;
    const int n = int(s.size());
    const vector<int> sa = suffix_array(s);
    vector<int> nxt0, nxt1;
    lyndon_hybrid::lyndon_ends_list(sa, nxt0, nxt1);
    vector<unsigned char> buf(n + 16, 0);
    memcpy(buf.data() + 8, s.data(), n);
    const unsigned char *t = buf.data() + 8;
    long long budget = 32LL * n;
    lyndon_hybrid::Slow slow{s, sa, nullptr, nullptr};
    ans = lyndon_runs::collect(nxt0, nxt1, [&](int i, int j, int p) __attribute__((always_inline)) {
      if (__builtin_expect(budget < 0, 0)) return slow(i, j, p);
      // 根の後ろの 8 バイトと手前の 8 バイト (j = n なら後ろは余白で、e は 0 になる)
      int e = countr_zero(load(t + i) ^ load(t + j)) >> 3, b = countl_zero(load(t + i - 8) ^ load(t + j - 8)) >> 3;
      if (__builtin_expect(e < 8 && b < 8, 1)) {
        e = min(e, n - j), b = min(b, i);
        return e + b >= p ? pair<int, int>(i - b, j + e) : pair<int, int>(-1, 0);
      }
      e = e < 8 ? min(e, n - j) : forward(t, i, j, 8, n - j);
      b = b < 8 ? min(b, i) : backward(t, i, j, 8, i, p - e);
      budget -= e + b;
      if (e + b < p) return pair<int, int>(-1, 0);
      if (b >= 8) {
        const int b0 = b;
        b = backward(t, i, j, b, i, i);
        budget -= b - b0;
      }
      return pair<int, int>(i - b, j + e);
    });
  }

  const vector<array<int, 3>> &answer() const { return ans; }
};
