#pragma once
#include "pj.hpp"
#ifdef __x86_64__
#include <immintrin.h>
#else
#include <simde/x86/avx2.h>
#endif
#include <algorithm>
#include <cstdint>
#include <memory>
#ifdef __linux__
#include <sys/mman.h>
#endif

// 子を 16 個持つ双対セグ木。段 0 は要素を 16 個ずつの塊に並べ、段 h >= 1 の塊は子 (段 h - 1 の
// 塊) 16 個ぶんの、まだ配っていない作用 (m, a) を並べて持つ。区間の作用は根から 2 本の道を
// 下り、端で一部だけ掛かる子へは作用を配ってから下りる。全部掛かる子へは、塊の中の連続した
// レーンへ AVX2 でまとめて合成する。根から葉への道では上の段ほど新しい作用になるので、1 点の
// 取得は葉の値に段 1 から順に作用を当てるだけ (n = 5 * 10^5 で 4 回、書き込みなし)。値は剰余を
// そのままの値で 2p 未満に持ち、掛け算は Shoup の方法で 8 レーンずつ。
namespace wide_dual {
constexpr u32 P = 998244353, P2 = 2 * P;

template <class T> struct HugeArray {
  unique_ptr<char[]> raw;
  T *p = nullptr;
  HugeArray() = default;
  explicit HugeArray(size_t n) {
    const size_t want = max<size_t>(n, 1) * sizeof(T);
    const size_t A = want >= (size_t(1) << 20) ? (size_t(1) << 21) : 64;
    const size_t bytes = (want + A - 1) & ~(A - 1);
    raw.reset(new char[bytes + A]);
    p = reinterpret_cast<T *>((reinterpret_cast<uintptr_t>(raw.get()) + A - 1) & ~uintptr_t(A - 1));
#ifdef __linux__
    if (A > 64) madvise(p, bytes, MADV_HUGEPAGE);
#endif
  }
};

struct Mul {
  __m256i m, q;
  explicit Mul(u32 x) {
    x = x >= P ? x - P : x;
    m = _mm256_set1_epi32((int)x);
    q = _mm256_set1_epi32((int)u32((u64(x) << 32) / P));
  }
};
inline __m256i mul(__m256i x, const Mul &f) {
  const __m256i e = _mm256_mul_epu32(x, f.q);
  const __m256i o = _mm256_mul_epu32(_mm256_srli_epi64(x, 32), f.q);
  const __m256i q = _mm256_blend_epi32(_mm256_srli_epi64(e, 32), o, 0xaa);
  return _mm256_sub_epi32(_mm256_mullo_epi32(x, f.m), _mm256_mullo_epi32(q, _mm256_set1_epi32((int)P)));
}
inline __m256i add(__m256i a, __m256i b) {
  const __m256i s = _mm256_add_epi32(a, b);
  return _mm256_min_epu32(s, _mm256_sub_epi32(s, _mm256_set1_epi32((int)P2)));
}
inline void masks(int lo, int hi, __m256i &k0, __m256i &k1) {
  const __m256i i0 = _mm256_setr_epi32(0, 1, 2, 3, 4, 5, 6, 7);
  const __m256i i1 = _mm256_setr_epi32(8, 9, 10, 11, 12, 13, 14, 15);
  const __m256i L = _mm256_set1_epi32(lo - 1), R = _mm256_set1_epi32(hi);
  k0 = _mm256_and_si256(_mm256_cmpgt_epi32(i0, L), _mm256_cmpgt_epi32(R, i0));
  k1 = _mm256_and_si256(_mm256_cmpgt_epi32(i1, L), _mm256_cmpgt_epi32(R, i1));
}
inline __m256i ld(const u32 *p) { return _mm256_load_si256((const __m256i *)p); }
inline void st(u32 *p, __m256i x) { _mm256_store_si256((__m256i *)p, x); }

struct alignas(64) Blk {
  u32 m[16], a[16];
};

class Tree {
  int n = 0, H = 0;
  u32 *v0 = nullptr;
  Blk *lv[8] = {};
  HugeArray<u32> mem0;
  HugeArray<Blk> mem1;

  template <bool ALL> void apply_lanes(int h, int b, int lo, int hi, u32 m, u32 a) {
    const Mul f(m);
    const __m256i av = _mm256_set1_epi32((int)a);
    __m256i k0 = _mm256_setzero_si256(), k1 = k0;
    if constexpr (!ALL) masks(lo, hi, k0, k1);
    auto upd = [&](u32 *p, __m256i x0, __m256i x1) {
      if constexpr (ALL) {
        st(p, x0), st(p + 8, x1);
      } else {
        st(p, _mm256_blendv_epi8(ld(p), x0, k0));
        st(p + 8, _mm256_blendv_epi8(ld(p + 8), x1, k1));
      }
    };
    if (h == 0) {
      u32 *p = v0 + (size_t(b) << 4);
      upd(p, add(mul(ld(p), f), av), add(mul(ld(p + 8), f), av));
      return;
    }
    Blk &B = lv[h][b];
    upd(B.m, mul(ld(B.m), f), mul(ld(B.m + 8), f));
    upd(B.a, add(mul(ld(B.a), f), av), add(mul(ld(B.a + 8), f), av));
  }
  void push(int h, int c) {
    Blk &B = lv[h][c >> 4];
    const int j = c & 15;
    const u32 m = B.m[j], a = B.a[j];
    if (m == 1 && a == 0) return;
    B.m[j] = 1, B.a[j] = 0;
    apply_lanes<true>(h - 1, c, 0, 16, m, a);
  }
  void apply_rec(int h, int b, int lo, int hi, u32 m, u32 a) {
    const int sh = 4 * h;
    const int jl = (lo >> sh) & 15, jr = ((hi - 1) >> sh) & 15;
    if (h == 0) return apply_lanes<false>(0, b, jl, jr + 1, m, a);
    const int sm = (1 << sh) - 1, base = b << 4;
    int fl = jl, fr = jr + 1;
    if (lo & sm) {
      const int c = base + jl, e = min(hi, (lo | sm) + 1);
      push(h, c);
      apply_rec(h - 1, c, lo, e, m, a);
      if (jl == jr) return;
      fl = jl + 1;
    }
    if (hi & sm) {
      const int c = base + jr, s = max(lo, hi & ~sm);
      push(h, c);
      apply_rec(h - 1, c, s, hi, m, a);
      fr = jr;
    }
    if (fl < fr) apply_lanes<false>(h, b, fl, fr, m, a);
  }

public:
  explicit Tree(const vector<i64> &a) : n((int)a.size()) {
    int cnt[8], nb = (n + 15) >> 4;
    H = 0;
    size_t total1 = 0;
    for (;;) {
      cnt[H++] = nb;
      if (nb == 1) break;
      nb = (nb + 15) >> 4;
      total1 += nb;
    }
    mem0 = HugeArray<u32>(size_t(cnt[0]) << 4);
    mem1 = HugeArray<Blk>(max<size_t>(total1, 1));
    v0 = mem0.p;
    size_t off = 0;
    for (int h = 1; h < H; ++h) lv[h] = mem1.p + off, off += cnt[h];
    for (int i = 0; i < (cnt[0] << 4); ++i) v0[i] = i < n ? u32(a[i] % P) : 0;
    for (int h = 1; h < H; ++h)
      for (int b = 0; b < cnt[h]; ++b)
        for (int j = 0; j < 16; ++j) lv[h][b].m[j] = 1, lv[h][b].a[j] = 0;
  }
  void affine(int l, int r, u32 m, u32 a) { apply_rec(H - 1, 0, l, r, m, a); }
  u32 get(int i) const {
    u32 x = v0[i];
    for (int h = 1; h < H; ++h) {
      const int c = i >> (4 * h);
      const Blk &B = lv[h][c >> 4];
      x = u32((u64(B.m[c & 15]) * x + B.a[c & 15]) % P);
    }
    return x >= P ? x - P : x;
  }
};
}  // namespace wide_dual

struct Solver {
  wide_dual::Tree t;
  explicit Solver(const vector<i64> &a) : t(a) {}
  void affine(int l, int r, i64 b, i64 c) { t.affine(l, r, (u32)b, (u32)c); }
  i64 get(int i) { return t.get(i); }
};
