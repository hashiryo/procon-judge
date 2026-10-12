#pragma once
// psl_fenwick と同じ seaweed の組み替えで h を 1 行ずつ求め、各行で、クエリを O(1) の手順で答えるための表を作って全部の行の分を持ち、
// クエリを入力の順に答える版。a 行のあとの答えは D(b, c) - b で、D(b, c) = #{j < c : h[j] < b} (j < b なら h[j] <= j < b なので
// #{j < b : h[j] < b} = b)。h[j] < b となる j の集合は、h[j] = -1 の位置に、x < b の糸 x が下端に出る位置 pos[x] を足したもの。
// 各行で b0 = 64 k (k = 0, 1, ...) ごとに、集合を 64 bit の語の列と各語より前の位置の数 cum で控えておくと、
// D(b, c) = cum[c / 64] + (語 c / 64 のうち c より下の popcount) + #{x in [b0, b) : pos[x] < c} (b0 = b を 64 の倍数に切り下げた値)
// で、最後の項は pos の 64 個を AVX2 の比較 4 回で数える。クエリに分岐の外れる繰り返しが無い。psl_scan は [b, c) の h を 16 個ずつ
// 数えていて、手元の M2 で 5 * 10^5 個のクエリに 12 ms かかった。表は 1 行あたり pos が 2 KB、控えが 4 KB で、2 MB 以上なら Linux
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

namespace psl_ckpt {
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
}  // namespace psl_ckpt
namespace psl_ckpt {
struct Entry {
  uint64_t word;
  uint32_t cum, pad;
};
constexpr int PW = 1024 + 64;  // pos の行の幅。x >= n と右端に出た糸は 1024 (どの c よりも大きい)、1087 番は h[j] = -1 の捨て場
}  // namespace psl_ckpt

struct Solver {
  string s, t;
  vector<array<int, 3>> qs;
  vector<int> ans;

  Solver(const string &s, const string &t, const vector<array<int, 3>> &qs) : s(s), t(t), qs(qs) {}

  void run() {
    using psl_ckpt::Entry, psl_ckpt::PW;
    const int m = int(s.size()), n = int(t.size()), q = int(qs.size());
    const int nk = (n >> 6) + 1;  // b0 の数 (b <= n)
    const size_t ck_row = size_t(nk) * 16;
    psl_ckpt::Zeroed buf(size_t(m + 1) * (PW * sizeof(int16_t) + ck_row * sizeof(Entry)));
    int16_t *pos = static_cast<int16_t *>(buf.get());
    Entry *ck = reinterpret_cast<Entry *>(pos + size_t(m + 1) * PW);
    vector<int16_t> h(1024, 1024);  // n 以降は 1024 で埋め、どの b0 よりも小さくならないようにする
    iota(h.begin(), h.begin() + n, int16_t(0));
    for (int a = 0; a <= m; ++a) {
      if (a > 0) {
        const char x = s[a - 1];
        int v = -1;
        for (int j = 0; j < n; ++j) {
          const int hj = h[j];
          const bool sw = (x == t[j]) | (hj < v);
          h[j] = int16_t(sw ? v : hj);
          v = sw ? hj : v;
        }
      }
      int16_t *pr = pos + size_t(a) * PW;
      for (int x = 0; x < PW; ++x) pr[x] = 1024;
      for (int j = 0; j < n; ++j) {
        const int hj = h[j];
        pr[hj >= 0 ? hj : PW - 1] = int16_t(j);
      }
      // 控え k の語 e の bit j は h[64 e + j] < 64 k。h の 64 個を 1 回読み、控えごとに比べて 64 bit にまとめる
      Entry *ca = ck + size_t(a) * ck_row;
      for (int e = 0; e < 16; ++e) {
        const __m256i *hp = reinterpret_cast<const __m256i *>(h.data() + 64 * e);
        const __m256i h0 = _mm256_loadu_si256(hp), h1 = _mm256_loadu_si256(hp + 1), h2 = _mm256_loadu_si256(hp + 2), h3 = _mm256_loadu_si256(hp + 3);
        for (int k = 0; k < nk; ++k) {
          const __m256i vb = _mm256_set1_epi16(int16_t(64 * k));
          const __m256i lo = _mm256_permute4x64_epi64(_mm256_packs_epi16(_mm256_cmpgt_epi16(vb, h0), _mm256_cmpgt_epi16(vb, h1)), 0xd8);
          const __m256i hi = _mm256_permute4x64_epi64(_mm256_packs_epi16(_mm256_cmpgt_epi16(vb, h2), _mm256_cmpgt_epi16(vb, h3)), 0xd8);
          ca[k * 16 + e].word = uint64_t(uint32_t(_mm256_movemask_epi8(lo))) | uint64_t(uint32_t(_mm256_movemask_epi8(hi))) << 32;
        }
      }
      for (int k = 0; k < nk; ++k)
        for (int e = 0, acc = 0; e < 16; ++e) ca[k * 16 + e].cum = uint32_t(acc), acc += popcount(ca[k * 16 + e].word);
    }
    ans.resize(q);
    const __m256i lane0 = _mm256_setr_epi16(0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15), l16 = _mm256_set1_epi16(16);
    for (int k = 0; k < q; ++k) {
      const auto [a, b, c] = qs[k];
      const int k0 = b >> 6, b0 = k0 << 6;
      const Entry e = ck[size_t(a) * ck_row + k0 * 16 + (c >> 6)];
      const int d = int(e.cum) + popcount(e.word & ((uint64_t(1) << (c & 63)) - 1));
      const int16_t *pp = pos + size_t(a) * PW + b0;
      const __m256i vc = _mm256_set1_epi16(int16_t(c)), vl = _mm256_set1_epi16(int16_t(b - b0));
      __m256i ln = lane0;
      int cnt = 0;
      for (int i = 0; i < 4; ++i) {
        const __m256i p = _mm256_loadu_si256(reinterpret_cast<const __m256i *>(pp + 16 * i));
        const __m256i in = _mm256_and_si256(_mm256_cmpgt_epi16(vc, p), _mm256_cmpgt_epi16(vl, ln));
        cnt += popcount(unsigned(_mm256_movemask_epi8(in)));
        ln = _mm256_add_epi16(ln, l16);
      }
      ans[k] = d + (cnt >> 1) - b;
    }
  }

  const vector<int> &answer() const { return ans; }
};
