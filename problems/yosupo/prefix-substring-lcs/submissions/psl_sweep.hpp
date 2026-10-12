#pragma once
// psl_fenwick と同じ seaweed の組み替えで h を 1 行ずつ求め、クエリを a ごとに分けてから a の中で b の順に並べ、b を 0 から n まで
// 進めながら答える版。a 行のあとで、h[j] < b となる j の集合は、h[j] = -1 の位置に、x < b の糸 x が下端に出る位置 pos[x] を足した
// ものなので、b を 1 つ進めるたびに増える位置は高々 1 つになる。集合を 64 bit の語の列 (n <= 1000 で 16 語) と、各語より前にある
// 位置の数 cum で持つと、[b, c) の位置の数は P(c) - P(b) (P(x) = cum[x / 64] + x より下の語の中の popcount) で O(1) で求まる。
// cum は 16 個の int16 なので、位置を足すときは AVX2 の 1 回の加算で更新する。並べ替えた (b, c, 番号) の列は 2 MB 以上なら、Linux
// では 2 MB 境界の mmap に置いて huge page を頼む。
// 接頭辞と部分文字列の LCS の実装を比べた記録は algo-notes の notes/prefix-substring-lcs.md にある。
#ifdef __x86_64__
#include <immintrin.h>
#else
#include <simde/x86/avx2.h>
#endif
#include <array>
#include <bit>
#include <cstdint>
#include <cstdlib>
#include <numeric>
#include <string>
#include <vector>
#ifdef __linux__
#include <sys/mman.h>
#endif
#include "pj.hpp"

namespace psl_sweep {
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
}  // namespace psl_sweep

struct Solver {
  string s, t;
  vector<array<int, 3>> qs;
  vector<int> ans;

  Solver(const string &s, const string &t, const vector<array<int, 3>> &qs) : s(s), t(t), qs(qs) {}

  void run() {
    const int m = int(s.size()), n = int(t.size()), q = int(qs.size());
    ans.assign(q, 0);  // a = 0 のクエリは 0 のまま
    struct Rec {
      int16_t b, c;
      int k;
    };
    psl_sweep::Zeroed buf(size_t(q) * 2 * sizeof(Rec));
    Rec *byA = static_cast<Rec *>(buf.get()), *byB = byA + q;
    vector<int> sa(m + 2, 0);
    for (const auto &x : qs) ++sa[x[0] + 1];
    for (int a = 0; a <= m; ++a) sa[a + 1] += sa[a];
    {
      vector<int> wp(sa.begin(), sa.end() - 1);
      for (int k = 0; k < q; ++k) byA[wp[qs[k][0]]++] = {int16_t(qs[k][1]), int16_t(qs[k][2]), k};
    }
    vector<int16_t> h(n), pos(n);
    iota(h.begin(), h.end(), int16_t(0));
    vector<int> cb(n + 2);
    alignas(32) int16_t cum[16];
    uint64_t word[17];
    const __m256i lane = _mm256_setr_epi16(0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15), one = _mm256_set1_epi16(1);
    for (int a = 1; a <= m; ++a) {
      const char x = s[a - 1];
      int v = -1;
      for (int j = 0; j < n; ++j) {
        const int hj = h[j];
        const bool sw = (x == t[j]) | (hj < v);
        h[j] = int16_t(sw ? v : hj);
        v = sw ? hj : v;
      }
      const int lo = sa[a], hi = sa[a + 1];
      if (lo == hi) continue;
      fill(cb.begin(), cb.end(), 0);
      for (int r = lo; r < hi; ++r) ++cb[byA[r].b + 1];
      for (int b = 0; b <= n; ++b) cb[b + 1] += cb[b];
      for (int r = lo; r < hi; ++r) byB[lo + cb[byA[r].b]++] = byA[r];
      // 右端に出た糸 x の pos[x] は 1024 (語 16 番、cum の 16 個のどれも増やさない) にして、足す手順を分岐なしにする
      fill(pos.begin(), pos.end(), int16_t(1024));
      for (int w = 0; w <= 16; ++w) word[w] = 0;
      for (int j = 0; j < n; ++j) {
        if (h[j] >= 0) pos[h[j]] = int16_t(j);
        else word[j >> 6] |= uint64_t(1) << (j & 63);
      }
      for (int w = 0, acc = 0; w < 16; ++w) cum[w] = int16_t(acc), acc += popcount(word[w]);
      auto prefix = [&](int y) { return cum[y >> 6] + popcount(word[y >> 6] & ((uint64_t(1) << (y & 63)) - 1)); };
      __m256i cv = _mm256_load_si256(reinterpret_cast<const __m256i *>(cum));
      int cur = 0;  // 集合は h[j] < cur の j
      for (int r = lo; r < hi; ++r) {
        const int b = byB[r].b;
        if (cur < b) {
          for (; cur < b; ++cur) {
            const int p = pos[cur];
            word[p >> 6] |= uint64_t(1) << (p & 63);
            cv = _mm256_add_epi16(cv, _mm256_and_si256(_mm256_cmpgt_epi16(lane, _mm256_set1_epi16(int16_t(p >> 6))), one));
          }
          _mm256_store_si256(reinterpret_cast<__m256i *>(cum), cv);
        }
        ans[byB[r].k] = prefix(byB[r].c) - prefix(b);
      }
    }
  }

  const vector<int> &answer() const { return ans; }
};
