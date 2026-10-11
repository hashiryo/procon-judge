#pragma once
// z_w8、z_w8b、z_w8b_hp が共有する中身。z_basic で、伸ばすところを 8 バイトずつ比べる。t[k, k + 8) と t[i + k, i + k + 8) の排他的
// 論理和の下位から 0 のバイトを数え、8 バイトとも一致したら次の 8 バイトへ進む。文字列は写さず、末尾の 8 バイトに満たないところだけ
// 1 文字ずつ比べる。First が true なら、8 バイトを読む前に 1 文字目だけを比べ、外れたらそこで止める (一様な文字列では、ほとんどの位置
// が 1 文字目で外れる)。z は呼ぶ側で確保しておく。
// Z 配列と回文の実装を比べた記録は algo-notes の notes/z-array-palindromes.md にある。
#include <bit>
#include <cstdint>
#include <cstring>

namespace z_w8 {
inline uint64_t load(const unsigned char *p) {
  uint64_t x;
  std::memcpy(&x, p, 8);
  return x;
}
template <bool First> inline void z_array(const unsigned char *t, int n, int *z) {
  if (n == 0) return;
  z[0] = n;
  for (int i = 1, l = 0, r = 0; i < n; ++i) {
    int k = 0;
    if (i < r) {
      k = z[i - l];
      if (k < r - i) {
        z[i] = k;
        continue;
      }
      k = r - i;
    }
    if (First && (i + k == n || t[k] != t[i + k])) {
      z[i] = k, l = i, r = i + k;
      continue;
    }
    for (;;) {
      if (i + k + 8 > n) {
        while (i + k < n && t[k] == t[i + k]) ++k;
        break;
      }
      if (const uint64_t x = load(t + k) ^ load(t + i + k)) {
        k += std::countr_zero(x) >> 3;
        break;
      }
      k += 8;
    }
    z[i] = k, l = i, r = i + k;
  }
}
}  // namespace z_w8
