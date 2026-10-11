#pragma once
// z_w8b で、出力の配列を 0 で埋める前に、その領域へ madvise(MADV_HUGEPAGE) を呼ぶ版 (Linux のみ)。OS は確保した領域の 4 KB ごとの
// ページに初めて書き込まれたところでメモリを割り当てるので、0 で埋めるのが初めての書き込みになる。runs の列挙の診断では、EPYC 7763
// で 4 KB のページ 1 枚あたり 1.7 us かかり、huge page (2 MB のページ) を使わせると 8 分の 1 ほどになった。std::vector の中身は
// 2 MB 境界にそろわないので、領域の中の 2 MB 境界にそろった部分にだけ huge page が使われる。
// Z 配列と回文の実装を比べた記録は algo-notes の notes/z-array-palindromes.md にある。
#include <cstdint>
#include <string>
#include <vector>
#ifdef __linux__
#include <sys/mman.h>
#endif
#include "pj.hpp"
#include "_z_w8.hpp"

struct Solver {
  string s;
  vector<int> z;

  explicit Solver(const string &s) : s(s) {}

  void run() {
    const size_t n = s.size();
    z.reserve(n);
#ifdef __linux__
    if (n * sizeof(int) >= (size_t(1) << 20)) {
      const uintptr_t a = reinterpret_cast<uintptr_t>(z.data()) & ~uintptr_t(4095);
      madvise(reinterpret_cast<void *>(a), reinterpret_cast<uintptr_t>(z.data()) + n * sizeof(int) - a, MADV_HUGEPAGE);
    }
#endif
    z.resize(n);
    z_w8::z_array<true>(reinterpret_cast<const unsigned char *>(s.data()), int(n), z.data());
  }

  const vector<int> &answer() const { return z; }
};
