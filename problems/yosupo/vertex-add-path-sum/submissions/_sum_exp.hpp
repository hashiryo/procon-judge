#pragma once
#include "pj.hpp"
#include "neo/data_structure/segtree/Sum.hpp"

// NeoLibrary の SegmentTree<monoid::Sum<i64>> (segtree/Sum.hpp の Wide) の写しで、作る段と prod を替えて比べるためのもの。
//   作る段: NeoLibrary は値の vector と u64 の vector を 1 本ずつ作って写してから段を作り、表を 0 で埋めてから書く。ここでは
//           段 0 を init から直接作り、段 1 から上だけを塊の合計の短い列 (n / 8 個) から作る。表は埋めずに確保する
//           (どの項目も作る段で書く)。
//   prod: EARLY が真なら、l と r が同じ子に入った段で止める (その段から上は同じ項目どうしの差で 0 になる)。
template <bool EARLY> class SumExp {
  using u64 = unsigned long long;
  static constexpr int LB = 3, B = 1 << LB;
  int n = 0, H = 0;
  size_t loff[24] = {}, total = 0;
  u64 *mem = nullptr;
  const u64 *L(int h) const { return mem + loff[h]; }
  u64 *L(int h) { return mem + loff[h]; }

 public:
  template <class F> SumExp(int n_, const F &init) : n(n_) {
    const size_t N = n;
    H = 1;
    while ((N >> (H * LB)) != 0) ++H;
    for (int h = 0; h < H; ++h) loff[h] = total, total += ((N >> ((h + 1) * LB)) + 1) << LB;
    mem = segment_tree_internal::Alloc<u64>().allocate(total);
    const size_t m0 = ((N >> LB) + 1) << LB;
    vector<u64> cur(m0 >> LB), nxt;
    u64 *l0 = L(0);
    for (size_t f = 0; f < m0; f += B) {
      u64 s = 0;
      for (int j = 0; j < B; ++j) {
        l0[f + j] = s;
        if (f + j < N) s += u64(init(int(f + j)));
      }
      cur[f >> LB] = s;
    }
    for (int h = 1; h < H; ++h) {
      u64 *lv = L(h);
      const size_t m = ((N >> ((h + 1) * LB)) + 1) << LB;
      nxt.assign(m >> LB, 0);
      for (size_t f = 0; f < m; ++f) {
        const size_t j = f & (B - 1);
        lv[f] = j ? lv[f - 1] + (f - 1 < cur.size() ? cur[f - 1] : 0) : 0;
        if (j == B - 1) nxt[f >> LB] = lv[f] + (f < cur.size() ? cur[f] : 0);
      }
      cur.swap(nxt);
    }
  }
  ~SumExp() {
    if (mem) segment_tree_internal::Alloc<u64>().deallocate(mem, total);
  }
  SumExp(const SumExp &) = delete;
  SumExp &operator=(const SumExp &) = delete;

  [[gnu::always_inline]] void mul(int p, i64 x) {
    const __m256i xv = _mm256_set1_epi64x(x);
    for (int h = 0; h < H; ++h) {
      const size_t c = size_t(p) >> (h * LB);
      u64 *blk = L(h) + ((c >> LB) << LB);
      const u64 *m = segtree_sum_internal::mask.v + (B - 1 - (c & (B - 1)));
      const __m256i v0 = _mm256_load_si256((const __m256i *)blk), v1 = _mm256_load_si256((const __m256i *)(blk + 4));
      const __m256i m0 = _mm256_loadu_si256((const __m256i *)m), m1 = _mm256_loadu_si256((const __m256i *)(m + 4));
      _mm256_store_si256((__m256i *)blk, _mm256_add_epi64(v0, _mm256_and_si256(m0, xv)));
      _mm256_store_si256((__m256i *)(blk + 4), _mm256_add_epi64(v1, _mm256_and_si256(m1, xv)));
    }
  }
  [[gnu::always_inline]] i64 prod(int l, int r) const {
    u64 s = 0;
    if constexpr (EARLY) {
      size_t a = size_t(r), b = size_t(l);
      for (int h = 0; a != b; ++h, a >>= LB, b >>= LB) s += L(h)[a] - L(h)[b];
    } else {
      for (int h = 0; h < H; ++h) s += L(h)[size_t(r) >> (h * LB)] - L(h)[size_t(l) >> (h * LB)];
    }
    return i64(s);
  }
};
