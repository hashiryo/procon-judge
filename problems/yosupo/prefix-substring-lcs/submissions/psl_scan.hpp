#pragma once
// psl_fenwick と同じ seaweed の組み替えで、各行のあとの h を全部 int16 で持ち (m = n = 1000 で 2 MB)、クエリを入力の順に、行 a の
// [b, c) で h[j] < b となる j の数を AVX2 で 16 個ずつ数える版。行 0 は h[j] = j で、どのクエリも 0 になる。クエリを並べ替えないので、
// a ごとに分ける数え分けと Fenwick 木が要らない。行の表は 2 MB 以上なら、Linux では 2 MB 境界の mmap に置いて huge page を頼む。
// 接頭辞と部分文字列の LCS の実装を比べた記録は algo-notes の notes/prefix-substring-lcs.md にある。
#ifdef __x86_64__
#include <immintrin.h>
#else
#include <simde/x86/avx2.h>
#endif
#include <array>
#include <cstdint>
#include <cstdlib>
#include <string>
#include <vector>
#ifdef __linux__
#include <sys/mman.h>
#endif
#include "pj.hpp"

namespace psl_scan {
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
}  // namespace psl_scan

struct Solver {
  string s, t;
  vector<array<int, 3>> qs;
  vector<int> ans;

  Solver(const string &s, const string &t, const vector<array<int, 3>> &qs) : s(s), t(t), qs(qs) {}

  void run() {
    const int m = int(s.size()), n = int(t.size()), q = int(qs.size());
    const int w = ((n + 15) & ~15) + 16;  // 行の幅。c - 1 から 16 個読んでも行の中に収まる
    psl_scan::Zeroed buf(size_t(m + 1) * w * sizeof(int16_t));
    int16_t *rows = static_cast<int16_t *>(buf.get());
    for (int j = 0; j < n; ++j) rows[j] = int16_t(j);
    for (int a = 0; a < m; ++a) {
      const int16_t *h = rows + size_t(a) * w;
      int16_t *g = rows + size_t(a + 1) * w;
      const char x = s[a];
      int v = -1;
      for (int j = 0; j < n; ++j) {
        const int hj = h[j];
        const bool sw = (x == t[j]) | (hj < v);
        g[j] = int16_t(sw ? v : hj);
        v = sw ? hj : v;
      }
    }
    ans.resize(q);
    const __m256i lane = _mm256_setr_epi16(0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15);
    for (int k = 0; k < q; ++k) {
      const auto [a, b, c] = qs[k];
      const int16_t *r = rows + size_t(a) * w;
      const __m256i vb = _mm256_set1_epi16(int16_t(b));
      __m256i acc = _mm256_setzero_si256();
      int j = b;
      for (; j + 16 <= c; j += 16) acc = _mm256_sub_epi16(acc, _mm256_cmpgt_epi16(vb, _mm256_loadu_si256(reinterpret_cast<const __m256i *>(r + j))));
      const __m256i in = _mm256_cmpgt_epi16(_mm256_set1_epi16(int16_t(c - j)), lane);
      acc = _mm256_sub_epi16(acc, _mm256_and_si256(in, _mm256_cmpgt_epi16(vb, _mm256_loadu_si256(reinterpret_cast<const __m256i *>(r + j)))));
      __m256i s32 = _mm256_madd_epi16(acc, _mm256_set1_epi16(1));
      __m128i x = _mm_add_epi32(_mm256_castsi256_si128(s32), _mm256_extracti128_si256(s32, 1));
      x = _mm_add_epi32(x, _mm_shuffle_epi32(x, 0x4e));
      x = _mm_add_epi32(x, _mm_shuffle_epi32(x, 0xb1));
      ans[k] = _mm_cvtsi128_si32(x);
    }
  }

  const vector<int> &answer() const { return ans; }
};
