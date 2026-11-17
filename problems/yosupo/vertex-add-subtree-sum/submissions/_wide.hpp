#pragma once
#include "pj.hpp"
#ifdef __x86_64__
#include <immintrin.h>
#else
#include <simde/x86/avx2.h>
#endif
#include <cstdint>
#include <memory>
#ifdef __linux__
#include <sys/mman.h>
#endif

// 節を B = 2^LB 個に広げた接頭辞和の木 (Algorithmica の wide segment tree、Library Checker
// の首位 nandhagk の提出と同じ考え方)。段 h の表の f 番目には、f と同じ塊 (B 個ずつ) の中で
// f より前にある子の和を持つ。段 h の子は段 h - 1 の塊で、要素 [g B^h, (g + 1) B^h) を覆う。
// そうすると [0, r) の和は、段ごとに表の (r >> h LB) 番目を 1 つずつ読んで足すだけになる。
// 1 点の加算は、段ごとに p を含む子より後ろの同じ塊の項目に x を足す。塊の B 個を AVX2 で
// まとめて、マスクを取った x を足す。段の数は n < B^H となる最小の H。
template <int B> struct WideMask {
  // v[B - 1 - s + j] は j > s のとき全部 1。s 番目より後ろのレーンを選ぶ窓。
  alignas(64) u64 v[2 * B];
  constexpr WideMask() : v{} {
    for (int j = 0; j < B; ++j) v[B + j] = ~0ull;
  }
};
template <int B> inline constexpr WideMask<B> wide_mask{};

template <int LB> struct WidePrefixSum {
  static constexpr int B = 1 << LB;
  static_assert(B >= 4);
  int n = 0, H = 0;
  u64 *lv[16] = {};
  unique_ptr<char[]> raw;

  explicit WidePrefixSum(const vector<i64> &a) : n((int)a.size()) {
    const size_t N = (size_t)n;
    H = 1;
    while ((N >> (H * LB)) != 0) ++H;
    size_t off[16], total = 0;
    for (int h = 0; h < H; ++h) {
      off[h] = total;
      total += ((N >> ((h + 1) * LB)) + 1) << LB;
    }
    u64 *t = alloc(total);
    for (int h = 0; h < H; ++h) lv[h] = t + off[h];
    vector<u64> pre(N + 1);
    pre[0] = 0;
    for (size_t i = 0; i < N; ++i) pre[i + 1] = pre[i] + (u64)a[i];
    for (int h = 0; h < H; ++h) {
      const size_t m = ((N >> ((h + 1) * LB)) + 1) << LB;
      for (size_t f = 0; f < m; ++f) {
        const size_t hi = min(N, f << (h * LB));
        const size_t lo = min(N, ((f >> LB) << LB) << (h * LB));
        lv[h][f] = pre[hi] - pre[lo];
      }
    }
  }

  u64 *alloc(size_t total) {
    const size_t want = total * sizeof(u64);
    const size_t A = want >= (size_t(1) << 20) ? (size_t(1) << 21) : 64;
    const size_t bytes = (want + A - 1) & ~(A - 1);
    raw.reset(new char[bytes + A]);
    u64 *t = reinterpret_cast<u64 *>((reinterpret_cast<uintptr_t>(raw.get()) + A - 1) & ~uintptr_t(A - 1));
#ifdef __linux__
    if (A > 64) madvise(t, bytes, MADV_HUGEPAGE);
#endif
    return t;
  }

  // [l, r) の和。2 つの接頭辞を同じループで読む。
  i64 sum(int l, int r) const {
    u64 s = 0;
    for (int h = 0; h < H; ++h) s += lv[h][size_t(r) >> (h * LB)] - lv[h][size_t(l) >> (h * LB)];
    return (i64)s;
  }

  void add(int p, i64 x) {
    const __m256i xv = _mm256_set1_epi64x(x);
    for (int h = 0; h < H; ++h) {
      const size_t c = size_t(p) >> (h * LB);
      u64 *blk = lv[h] + ((c >> LB) << LB);
      const u64 *m = wide_mask<B>.v + (B - 1 - (c & (B - 1)));
      for (int k = 0; k < B; k += 4) {
        __m256i v = _mm256_load_si256((const __m256i *)(blk + k));
        const __m256i mk = _mm256_loadu_si256((const __m256i *)(m + k));
        v = _mm256_add_epi64(v, _mm256_and_si256(mk, xv));
        _mm256_store_si256((__m256i *)(blk + k), v);
      }
    }
  }
};
