#pragma once
// nos_adapt で、作業の配列を 1 つの 0 で埋まった領域 (2 MB 以上なら、Linux では 2 MB 境界の mmap に置いて huge page を頼む) に置き、
// 基数ソートの数え上げを 1 回の走査にまとめた版。nos_adapt は幅の広い列で、(値, 添字) の組の 2 本 (n = 5 * 10^5 で 8 MB)、前の位置
// (2 MB)、DP の値 (2 MB) を 4 KB のページに取っていて、1 回目 (1207ae1d と d314fa66 の run 38211024539、EPYC 9V45) の x64-gcc で
// wide_max が 6.41 ms だった。確保したばかりの領域に初めて書く代価は、EPYC 7763 で 1 MB あたり 0.43 ms なので、12 MB なら 5 ms ほど
// になる。ここでは前の位置と DP の値を、並べ終えた側でない組の列の領域に置く。幅の狭い列の表も同じ形の領域に置く。
// 異なる部分列の数え上げの実装を比べた記録は algo-notes の notes/number-of-subsequences.md にある。
#include <algorithm>
#include <array>
#include <cstdint>
#include <cstdlib>
#include <utility>
#include <vector>
#ifdef __linux__
#include <sys/mman.h>
#endif
#include "pj.hpp"

namespace nos_adapt2 {
constexpr unsigned P = 998244353;
// bytes byte の 0 で埋まった領域。2 MB 以上なら、Linux では 2 MB 境界の mmap に置いて huge page を頼む。
class Zeroed {
  void *p = nullptr;
#ifdef __linux__
  size_t mapped = 0;
#endif

 public:
  explicit Zeroed(size_t bytes) {
#ifdef __linux__
    constexpr size_t H = size_t(1) << 21;
    if (bytes >= H) {
      const size_t b = (bytes + 4095) & ~size_t(4095);
      void *m = mmap(nullptr, b + H, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
      if (m != MAP_FAILED) {
        char *q = static_cast<char *>(m), *a = reinterpret_cast<char *>((reinterpret_cast<uintptr_t>(q) + H - 1) & ~(H - 1));
        if (a != q) munmap(q, a - q);
        munmap(a + b, q + H - a);
        madvise(a, b, MADV_HUGEPAGE);
        p = a, mapped = b;
        return;
      }
    }
#endif
    p = calloc(bytes ? bytes : 1, 1);
  }
  ~Zeroed() {
#ifdef __linux__
    if (mapped) {
      munmap(p, mapped);
      return;
    }
#endif
    free(p);
  }
  Zeroed(const Zeroed &) = delete;
  Zeroed &operator=(const Zeroed &) = delete;
  void *get() const { return p; }
};
}  // namespace nos_adapt2

struct Solver {
  vector<unsigned> a;
  unsigned ans = 0;

  explicit Solver(const vector<unsigned> &a) : a(a) {}

  void run() {
    using nos_adapt2::P;
    const int n = int(a.size());
    unsigned mn = ~0u, mx = 0;
    for (const unsigned x : a) mn = min(mn, x), mx = max(mx, x);
    const uint64_t range = uint64_t(mx) - mn + 1;
    if (range <= max<uint64_t>(2 * uint64_t(n), 1 << 16)) {
      // 値から直接引く表に、その値が最後に出たときの DP の値 + 1 を持つ (0 は「まだ出ていない」)
      nos_adapt2::Zeroed buf(range * sizeof(unsigned));
      unsigned *last = static_cast<unsigned *>(buf.get());
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
    nos_adapt2::Zeroed buf(size_t(n) * 2 * sizeof(uint64_t));
    uint64_t *p = static_cast<uint64_t *>(buf.get()), *q = p + n;  // 上位 32 bit が値 - 最小、下位 32 bit が添字
    array<array<int, 2048>, 3> cnt{};
    for (int i = 0; i < n; ++i) {
      const unsigned v = a[i] - mn;
      p[i] = uint64_t(v) << 32 | unsigned(i);
      ++cnt[0][v & 2047], ++cnt[1][(v >> 11) & 2047], ++cnt[2][v >> 22];
    }
    for (int j = 0; j < 3; ++j) {
      const int sh = 11 * j;
      if (!((diff >> sh) & 2047)) continue;
      auto &c = cnt[j];
      for (int b = 0, acc = 0; b < 2048; ++b) {
        const int t = c[b];
        c[b] = acc, acc += t;
      }
      for (int i = 0; i < n; ++i) q[c[(p[i] >> 32 >> sh) & 2047]++] = p[i];
      swap(p, q);
    }
    // 並べ終えた p でない側 q の領域に、前の位置 prev (n 個) と DP の値 D (D[i] は a[0, i) の空も含めた相異なる部分列の数、i < n) を置く
    int *prev = reinterpret_cast<int *>(q);
    unsigned *D = reinterpret_cast<unsigned *>(q) + n;
    prev[unsigned(p[0])] = -1;
    for (int k = 1; k < n; ++k) prev[unsigned(p[k])] = (p[k] >> 32) == (p[k - 1] >> 32) ? int(unsigned(p[k - 1])) : -1;
    unsigned d = 1;
    D[0] = 1;
    for (int i = 0; i < n; ++i) {
      unsigned x = d * 2;
      x -= x >= P ? P : 0;
      if (prev[i] >= 0) {
        x += P - D[prev[i]];
        x -= x >= P ? P : 0;
      }
      d = x;
      if (i + 1 < n) D[i + 1] = d;
    }
    ans = d == 0 ? P - 1 : d - 1;
  }

  unsigned answer() const { return ans; }
};
