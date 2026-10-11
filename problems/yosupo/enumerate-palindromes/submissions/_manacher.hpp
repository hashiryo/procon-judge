#pragma once
// manacher_basic と manacher_w8b が共有する中身。2n - 1 個の中心 c を 1 本のループで回す Manacher。中心 c の回文を [lo, hi) とすると
// lo + hi = c + 1 なので、右端 hi だけで決まる (長さは 2 hi - c - 1)。右端が最も右にある回文 (箱) の中心 C と右端 H を持ち、
// c / 2 + 1 < H なら、C について対称な中心 c' = 2C - c の回文の右端 h' から、hm = c - C + h' までは回文だと分かる。hm < H なら
// それが答えで比べは要らず、そうでなければ H から伸ばす。W8 が true なら、伸ばすときに 1 文字目を比べてから、右側の 8 バイトと、
// 左側の 8 バイトをバイトの順で反転したものの排他的論理和の下位から 0 のバイトを数える (文字列は写さず、両端の 8 バイトに満たない
// ところだけ 1 文字ずつ比べる)。len は呼ぶ側で確保しておく。
// Z 配列と回文の実装を比べた記録は algo-notes の notes/z-array-palindromes.md にある。
#include <bit>
#include <cstdint>
#include <cstring>

namespace manacher {
inline uint64_t load(const unsigned char *p) {
  uint64_t x;
  std::memcpy(&x, p, 8);
  return x;
}
template <bool W8> inline void palindromes(const unsigned char *t, int n, int *len) {
  for (int c = 0, C = 0, H = 0; c < 2 * n - 1; ++c) {
    int h = c / 2 + 1;  // 最も短い回文の右端 (c が偶数なら 1 文字、奇数なら空)
    if (h < H) {
      const int c2 = 2 * C - c, hm = c - C + ((len[c2] + c2 + 1) >> 1);
      if (hm < H) {
        len[c] = 2 * hm - c - 1;
        continue;
      }
      h = H;
    }
    int lo = c + 1 - h;
    if constexpr (W8) {
      if (lo > 0 && h < n && t[lo - 1] == t[h]) {
        --lo, ++h;
        for (;;) {
          if (lo < 8 || h + 8 > n) {
            while (lo > 0 && h < n && t[lo - 1] == t[h]) --lo, ++h;
            break;
          }
          if (const uint64_t x = load(t + h) ^ __builtin_bswap64(load(t + lo - 8))) {
            const int m = std::countr_zero(x) >> 3;
            lo -= m, h += m;
            break;
          }
          lo -= 8, h += 8;
        }
      }
    } else {
      while (lo > 0 && h < n && t[lo - 1] == t[h]) --lo, ++h;
    }
    len[c] = h - lo;
    C = c, H = h;
  }
}
}  // namespace manacher
