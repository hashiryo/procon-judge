#pragma once
// 値の幅で 2 つの道を分ける版。値の幅 (最大 - 最小 + 1) が要素の数の 2 倍か 2^16 の大きい方以下なら、値から直接引く表に、その値が
// 最後に出たときの DP の値 + 1 を持ち (0 は「まだ出ていない」)、1 回の走査で dp' = 2 dp - (前の dp) を求める (nos_hash の表を配列に
// したもの)。幅が広ければ nos_radix と同じく (値 - 最小, 添字) を 11 bit ずつの基数ソートで並べて前の位置を求めるが、全要素で同じに
// なる桁の段 (値の OR と AND で分かる) は飛ばす。nos_radix は、すべて同じ値の列で各段の数え口が 1 つに集まり、加算の待ちが連なって
// 遅かった (手元の M2 で並べる段が 6.71 ms、値がばらばらなら 2.47 ms)。DP は _nos_dp.hpp。
// 異なる部分列の数え上げの実装を比べた記録は algo-notes の notes/number-of-subsequences.md にある。
#include <algorithm>
#include <array>
#include <bit>
#include <cstdint>
#include <vector>
#include "pj.hpp"
#include "_nos_dp.hpp"

struct Solver {
  vector<unsigned> a;
  unsigned ans = 0;

  explicit Solver(const vector<unsigned> &a) : a(a) {}

  void run() {
    constexpr unsigned P = nos_dp::P;
    const int n = int(a.size());
    unsigned mn = ~0u, mx = 0;
    for (const unsigned x : a) mn = min(mn, x), mx = max(mx, x);
    const uint64_t range = uint64_t(mx) - mn + 1;
    if (range <= max<uint64_t>(2 * uint64_t(n), 1 << 16)) {
      vector<unsigned> last(range, 0);
      unsigned d = 1;
      for (const unsigned x : a) {
        unsigned &r = last[x - mn];
        const unsigned before = d;
        d *= 2;
        d -= d >= P ? P : 0;
        if (r) {
          d += P - (r - 1);
          d -= d >= P ? P : 0;
        }
        r = before + 1;
      }
      ans = d == 0 ? P - 1 : d - 1;
      return;
    }
    unsigned all_or = 0, all_and = ~0u;
    for (const unsigned x : a) all_or |= x - mn, all_and &= x - mn;
    const unsigned diff = all_or ^ all_and;  // 立っている桁だけが並べる意味を持つ
    vector<uint64_t> p(n), q(n);  // 上位 32 bit が値 - 最小、下位 32 bit が添字
    for (int i = 0; i < n; ++i) p[i] = uint64_t(a[i] - mn) << 32 | unsigned(i);
    for (int sh = 0; sh < 32; sh += 11) {
      if (!((diff >> sh) & 2047)) continue;
      array<int, 2049> cnt{};
      for (int i = 0; i < n; ++i) ++cnt[((p[i] >> 32 >> sh) & 2047) + 1];
      for (int c = 0; c < 2048; ++c) cnt[c + 1] += cnt[c];
      for (int i = 0; i < n; ++i) q[cnt[(p[i] >> 32 >> sh) & 2047]++] = p[i];
      p.swap(q);
    }
    vector<int> prev(n, -1);
    for (int k = 1; k < n; ++k)
      if ((p[k] >> 32) == (p[k - 1] >> 32)) prev[unsigned(p[k])] = int(unsigned(p[k - 1]));
    ans = nos_dp::count(prev);
  }

  unsigned answer() const { return ans; }
};
