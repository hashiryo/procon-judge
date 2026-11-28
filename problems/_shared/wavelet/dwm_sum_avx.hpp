#pragma once
// 重み付きの動的 wavelet matrix (x64 は AVX2 のマスク和)。各段の 0/1 の列の各 bit に重み (値そのもの) を持たせ、
// 区間の k 番目、x 未満の個数、x 未満の重みの和、区間の重みの和を返す。
// 動的ビット列は 256 bit の塊を並べ、塊の列の上に Fenwick を載せる (dwm.hpp の BlockBitVector と同じ形)。
// 塊は bit と重みの配列、語の境目ごとの 0/1 の重みの累積を持つ。
#include <cassert>
#include <cstdint>
#include <cstring>
#include <utility>
#if defined(__x86_64__)
#include <immintrin.h>
#define DWM_SUM_AVX2 1
#else
#define DWM_SUM_AVX2 0
#endif
#include <vector>

namespace dwm_sum_avx_internal {
using u32 = unsigned;
using u64 = unsigned long long;
using i64 = long long;

inline u64 low_mask(int k) {
  if (k <= 0) return 0;
  if (k >= 64) return ~u64(0);
  return (u64(1) << k) - 1;
}

struct WeightedBlockBitVectorAVX {
  static constexpr int W = 4;
  static constexpr int CAP = 64 * W;

  struct Block {
    u64 w[W];
    i64 wt[CAP];
    i64 p0[W], p1[W];  // 語の境目 [0, 64*i) の 0/1 の重みの和
    i64 tot0, tot1;    // 塊全体の 0/1 の重みの和
    int len;
  };

  std::vector<Block> blk;
  std::vector<int> fcnt, fones;
  std::vector<i64> fsum0, fsum1;
  int n = 0;

  WeightedBlockBitVectorAVX() = default;
  int size() const { return n; }

  static int pop_ones(const Block &b) {
    int c = 0;
    for (int i = 0; i < W; ++i) c += __builtin_popcountll(b.w[i] & low_mask(b.len - 64 * i));
    return c;
  }
  static int prefix_ones(const Block &b, int off) {
    int c = 0, i = 0;
    for (; i + 64 <= off; i += 64) c += __builtin_popcountll(b.w[i / 64]);
    if (i < off) c += __builtin_popcountll(b.w[i / 64] & low_mask(off - i));
    return c;
  }
  static bool get_bit(const Block &b, int off) { return (b.w[off >> 6] >> (off & 63)) & 1; }

  static void rebuild_block(Block &b) {
    i64 c0 = 0, c1 = 0;
    for (int i = 0; i < W; ++i) {
      b.p0[i] = c0;
      b.p1[i] = c1;
      int from = 64 * i, to = 64 * (i + 1);
      if (to > b.len) to = b.len;
      for (int j = from; j < to; ++j) {
        if (get_bit(b, j)) c1 += b.wt[j];
        else c0 += b.wt[j];
      }
    }
    b.tot0 = c0;
    b.tot1 = c1;
  }

  void rebuild() {
    int m = (int)blk.size();
    fcnt.assign(m + 1, 0);
    fones.assign(m + 1, 0);
    fsum0.assign(m + 1, 0);
    fsum1.assign(m + 1, 0);
    for (int i = 0; i < m; ++i) {
      rebuild_block(blk[i]);
      fcnt[i + 1] = blk[i].len;
      fones[i + 1] = pop_ones(blk[i]);
      fsum0[i + 1] = blk[i].tot0;
      fsum1[i + 1] = blk[i].tot1;
    }
    for (int i = 1; i <= m; ++i) {
      int j = i + (i & -i);
      if (j <= m) fcnt[j] += fcnt[i], fones[j] += fones[i], fsum0[j] += fsum0[i], fsum1[j] += fsum1[i];
    }
  }

  void find(int k, int &b, int &off, int &acc_ones) const {
    int m = (int)blk.size();
    if (k >= n) {
      b = m;
      off = 0;
      acc_ones = fcnt.empty() ? 0 : [&] {
        int s = 0;
        for (int i = m; i > 0; i -= i & -i) s += fones[i];
        return s;
      }();
      return;
    }
    b = 0;
    int acc_cnt = 0;
    acc_ones = 0;
    int step = 1;
    while ((step << 1) <= m) step <<= 1;
    for (; step; step >>= 1) {
      int nb = b + step;
      if (nb <= m && acc_cnt + fcnt[nb] <= k) {
        b = nb;
        acc_cnt += fcnt[b];
        acc_ones += fones[b];
      }
    }
    off = k - acc_cnt;
  }

  i64 fenwick_sum(const std::vector<i64> &f, int b) const {
    i64 s = 0;
    for (int i = b; i > 0; i -= i & -i) s += f[i];
    return s;
  }
  int fenwick_sum_int(const std::vector<int> &f, int b) const {
    int s = 0;
    for (int i = b; i > 0; i -= i & -i) s += f[i];
    return s;
  }

  int rank1(int k) const {
    int b, off, acc;
    find(k, b, off, acc);
    if (b == (int)blk.size()) return acc;
    return acc + prefix_ones(blk[b], off);
  }

  std::pair<bool, int> access_rank(int k) const {
    int b, off, acc;
    find(k, b, off, acc);
    assert(b < (int)blk.size());
    return {get_bit(blk[b], off), acc + prefix_ones(blk[b], off)};
  }

  // 塊 b の [0, off) の 0/1 の重みの和。
#if DWM_SUM_AVX2
  static i64 sum_mask_avx2(const i64 *wt, u64 mask, int lim) {
    alignas(32) static constexpr i64 tab[16][4] = {
        {0, 0, 0, 0},   {-1, 0, 0, 0},  {0, -1, 0, 0},  {-1, -1, 0, 0},
        {0, 0, -1, 0},  {-1, 0, -1, 0}, {0, -1, -1, 0}, {-1, -1, -1, 0},
        {0, 0, 0, -1},  {-1, 0, 0, -1}, {0, -1, 0, -1}, {-1, -1, 0, -1},
        {0, 0, -1, -1}, {-1, 0, -1, -1}, {0, -1, -1, -1}, {-1, -1, -1, -1}};
    __m256i acc = _mm256_setzero_si256();
    int i = 0;
    for (; i + 4 <= lim; i += 4) {
      int idx = (int)((mask >> i) & 15);
      __m256i m = _mm256_load_si256((const __m256i *)tab[idx]);
      __m256i v = _mm256_loadu_si256((const __m256i *)(wt + i));
      acc = _mm256_add_epi64(acc, _mm256_and_si256(v, m));
    }
    __m128i lo = _mm256_castsi256_si128(acc);
    __m128i hi = _mm256_extracti128_si256(acc, 1);
    __m128i s2 = _mm_add_epi64(lo, hi);
    i64 s = _mm_cvtsi128_si64(s2) + _mm_extract_epi64(s2, 1);
    for (; i < lim; ++i) s += wt[i] & (-(i64)((mask >> i) & 1));
    return s;
  }
#endif

  i64 prefix0(const Block &b, int off) const {
    int word = off >> 6;
    if (word == W) return b.tot0;
    int base = word * 64, lim = off - base;
    u64 mask = ~b.w[word];
#if DWM_SUM_AVX2
    return b.p0[word] + sum_mask_avx2(b.wt + base, mask, lim);
#else
    i64 s = b.p0[word];
    for (int i = 0; i < lim; ++i) s += b.wt[base + i] & (-(i64)((mask >> i) & 1));
    return s;
#endif
  }
  i64 prefix1(const Block &b, int off) const {
    int word = off >> 6;
    if (word == W) return b.tot1;
    int base = word * 64, lim = off - base;
    u64 mask = b.w[word];
#if DWM_SUM_AVX2
    return b.p1[word] + sum_mask_avx2(b.wt + base, mask, lim);
#else
    i64 s = b.p1[word];
    for (int i = 0; i < lim; ++i) s += b.wt[base + i] & (-(i64)((mask >> i) & 1));
    return s;
#endif
  }
  i64 sum0_prefix(int k) const {
    int b, off, acc;
    find(k, b, off, acc);
    if (b == (int)blk.size()) return fenwick_sum(fsum0, b);
    return fenwick_sum(fsum0, b) + prefix0(blk[b], off);
  }
  i64 sum1_prefix(int k) const {
    int b, off, acc;
    find(k, b, off, acc);
    if (b == (int)blk.size()) return fenwick_sum(fsum1, b);
    return fenwick_sum(fsum1, b) + prefix1(blk[b], off);
  }
  i64 sum_all_prefix(int k) const { return sum0_prefix(k) + sum1_prefix(k); }
  i64 sum0(int l, int r) const { return sum0_prefix(r) - sum0_prefix(l); }
  i64 sum1(int l, int r) const { return sum1_prefix(r) - sum1_prefix(l); }
  i64 sum_all(int l, int r) const { return sum_all_prefix(r) - sum_all_prefix(l); }

  static void insert_bit(Block &b, int off, bool bit, i64 w) {
    int word = off >> 6, sh = off & 63, old_len = b.len;
    bool cross_bit[W] = {};
    i64 cross_w[W] = {};
    for (int i = word + 1; i < W; ++i) {
      int pos = 64 * i - 1;
      if (pos < old_len) {
        cross_bit[i] = get_bit(b, pos);
        cross_w[i] = b.wt[pos];
      }
    }
    if (b.len > off) std::memmove(b.wt + off + 1, b.wt + off, (std::size_t)(b.len - off) * sizeof(i64));
    for (int i = W - 1; i > word; --i) b.w[i] = (b.w[i] << 1) | (b.w[i - 1] >> 63);
    u64 lo = b.w[word] & low_mask(sh);
    u64 hi = b.w[word] >> sh;
    b.w[word] = lo | (u64(bit) << sh) | (sh + 1 < 64 ? hi << (sh + 1) : 0);
    b.wt[off] = w;
    ++b.len;
    for (int i = word + 1; i < W; ++i) {
      if (bit) b.p1[i] += w;
      else b.p0[i] += w;
      if (cross_w[i]) {
        if (cross_bit[i]) b.p1[i] -= cross_w[i];
        else b.p0[i] -= cross_w[i];
      }
    }
  }
  static void erase_bit(Block &b, int off) {
    int word = off >> 6, sh = off & 63, old_len = b.len;
    bool bit = get_bit(b, off);
    i64 w = b.wt[off];
    bool enter_bit[W] = {};
    i64 enter_w[W] = {};
    for (int i = word + 1; i < W; ++i) {
      int pos = 64 * i;
      if (pos < old_len) {
        enter_bit[i] = get_bit(b, pos);
        enter_w[i] = b.wt[pos];
      }
    }
    b.w[word] = (b.w[word] & low_mask(sh)) | ((sh + 1 < 64 ? (b.w[word] >> (sh + 1)) : 0) << sh);
    if (word + 1 < W) b.w[word] |= (b.w[word + 1] & 1) << 63;
    for (int i = word + 1; i < W; ++i) {
      b.w[i] >>= 1;
      if (i + 1 < W) b.w[i] |= (b.w[i + 1] & 1) << 63;
    }
    if (off + 1 < b.len) std::memmove(b.wt + off, b.wt + off + 1, (std::size_t)(b.len - off - 1) * sizeof(i64));
    --b.len;
    b.w[W - 1] &= low_mask(b.len - 64 * (W - 1));
    for (int i = word + 1; i < W; ++i) {
      if (bit) b.p1[i] -= w;
      else b.p0[i] -= w;
      if (enter_w[i]) {
        if (enter_bit[i]) b.p1[i] += enter_w[i];
        else b.p0[i] += enter_w[i];
      }
    }
  }

  void split_block(int b) {
    Block old = blk[b];
    Block lo{}, hi{};
    lo.len = 128;
    hi.len = old.len - 128;
    lo.w[0] = old.w[0];
    lo.w[1] = old.w[1];
    hi.w[0] = old.w[2];
    hi.w[1] = old.w[3];
    for (int i = 0; i < 128; ++i) lo.wt[i] = old.wt[i];
    for (int i = 128; i < old.len; ++i) hi.wt[i - 128] = old.wt[i];
    rebuild_block(lo);
    rebuild_block(hi);
    blk[b] = lo;
    blk.insert(blk.begin() + b + 1, hi);
    rebuild();
  }

  int insert_rank(int k, bool bit, i64 w) {
    if (k == n) {
      if (blk.empty()) {
        blk.push_back(Block{});
        blk[0].len = 0;
        rebuild();
      }
      int b = (int)blk.size() - 1;
      if (blk[b].len == CAP) {
        split_block(b);
        ++b;
      }
      int ones_before = rank1(n);
      insert_bit(blk[b], blk[b].len, bit, w);
      ++n;
      for (int i = b + 1; i <= (int)blk.size(); i += i & -i) {
        fcnt[i] += 1;
        fones[i] += bit;
        if (bit) fsum1[i] += w;
        else fsum0[i] += w;
      }
      return ones_before;
    }
    int b, off, acc;
    find(k, b, off, acc);
    if (blk[b].len == CAP) {
      split_block(b);
      find(k, b, off, acc);
    }
    int ones_before = acc + prefix_ones(blk[b], off);
    insert_bit(blk[b], off, bit, w);
    ++n;
    for (int i = b + 1; i <= (int)blk.size(); i += i & -i) {
      fcnt[i] += 1;
      fones[i] += bit;
      (bit ? fsum1 : fsum0)[i] += w;
    }
    return ones_before;
  }

  struct EraseRes {
    bool bit;
    int ones_before;
    i64 weight;
  };
  EraseRes erase_rank(int k) {
    int b, off, acc;
    find(k, b, off, acc);
    bool bit = get_bit(blk[b], off);
    int ones_before = acc + prefix_ones(blk[b], off);
    i64 w = blk[b].wt[off];
    erase_bit(blk[b], off);
    --n;
    for (int i = b + 1; i <= (int)blk.size(); i += i & -i) {
      fcnt[i] -= 1;
      fones[i] -= bit;
      if (bit) fsum1[i] -= w;
      else fsum0[i] -= w;
    }
    if (blk[b].len == 0) {
      blk.erase(blk.begin() + b);
      rebuild();
    } else if (blk[b].len < 64 && b + 1 < (int)blk.size() && blk[b].len + blk[b + 1].len <= CAP) {
      int add = blk[b + 1].len;
      for (int i = 0; i < add; ++i) insert_bit(blk[b], blk[b].len, get_bit(blk[b + 1], i), blk[b + 1].wt[i]);
      blk.erase(blk.begin() + b + 1);
      rebuild();
    }
    return {bit, ones_before, w};
  }
};

template <int L>
struct WeightedWaveletMatrixAVX {
  WeightedBlockBitVectorAVX bv[L];
  int mid[L] = {};
  int n = 0;

  WeightedWaveletMatrixAVX() = default;

  template <class V>
  explicit WeightedWaveletMatrixAVX(const std::vector<V> &a) {
    n = (int)a.size();
    std::vector<i64> cur(n), nxt(n), zero(n), one(n);
    for (int i = 0; i < n; ++i) cur[i] = (i64)a[i];
    for (int h = L - 1; h >= 0; --h) {
      int z = 0, o = 0;
      for (int i = 0; i < n; ++i) {
        bool b = (cur[i] >> h) & 1;
        bv[h].insert_rank(i, b, cur[i]);
        if (b) one[o++] = cur[i];
        else zero[z++] = cur[i];
      }
      mid[h] = z;
      for (int i = 0; i < z; ++i) nxt[i] = zero[i];
      for (int i = 0; i < o; ++i) nxt[z + i] = one[i];
      cur.swap(nxt);
    }
  }

  int size() const { return n; }

  void insert(int k, i64 x) {
    for (int h = L - 1; h >= 0; --h) {
      bool b = (x >> h) & 1;
      int o = bv[h].insert_rank(k, b, x);
      if (b) k = mid[h] + o;
      else k -= o, ++mid[h];
    }
    ++n;
  }

  i64 erase_value(int k) {
    i64 res = 0;
    for (int h = L - 1; h >= 0; --h) {
      auto r = bv[h].erase_rank(k);
      if (r.bit) res |= i64(1) << h, k = mid[h] + r.ones_before;
      else k -= r.ones_before, --mid[h];
    }
    --n;
    return res;
  }

  void erase(int k) { (void)erase_value(k); }

  void set(int k, i64 x) {
    erase_value(k);
    insert(k, x);
  }

  i64 access(int k) const {
    i64 res = 0;
    for (int h = L - 1; h >= 0; --h) {
      auto [b, o] = bv[h].access_rank(k);
      if (b) res |= i64(1) << h, k = mid[h] + o;
      else k -= o;
    }
    return res;
  }

  i64 kth(int l, int r, int k) const {
    i64 res = 0;
    for (int h = L - 1; h >= 0; --h) {
      int ol = bv[h].rank1(l), orr = bv[h].rank1(r);
      int z = (r - l) - (orr - ol);
      if (k < z) {
        l -= ol;
        r -= orr;
      } else {
        k -= z;
        res |= i64(1) << h;
        l = mid[h] + ol;
        r = mid[h] + orr;
      }
    }
    return res;
  }

  int count_lt(int l, int r, i64 x) const {
    int c = 0;
    for (int h = L - 1; h >= 0; --h) {
      int ol = bv[h].rank1(l), orr = bv[h].rank1(r);
      if ((x >> h) & 1) {
        c += (r - l) - (orr - ol);
        l = mid[h] + ol;
        r = mid[h] + orr;
      } else {
        l -= ol;
        r -= orr;
      }
    }
    return c;
  }

  i64 sum_lt(int l, int r, i64 x) const {
    i64 s = 0;
    for (int h = L - 1; h >= 0; --h) {
      int ol = bv[h].rank1(l), orr = bv[h].rank1(r);
      if ((x >> h) & 1) {
        s += bv[h].sum0(l, r);
        l = mid[h] + ol;
        r = mid[h] + orr;
      } else {
        l -= ol;
        r -= orr;
      }
    }
    return s;
  }

  i64 sum_all(int l, int r) const { return bv[L - 1].sum_all(l, r); }

  // [l, r) の大きい方から食べて和が k 以上になる最小個数。総和が k 未満なら -1。
  int min_count_ge(int l, int r, i64 k) const {
    if (sum_all(l, r) < k) return -1;
    int cnt = 0;
    i64 v = 0;
    for (int h = L - 1; h >= 0; --h) {
      int ol = bv[h].rank1(l), orr = bv[h].rank1(r);
      i64 s1 = bv[h].sum1(l, r);
      int c1 = orr - ol;
      if (s1 >= k) {
        v |= i64(1) << h;
        l = mid[h] + ol;
        r = mid[h] + orr;
      } else {
        k -= s1;
        cnt += c1;
        l -= ol;
        r -= orr;
      }
    }
    if (k > 0) cnt += (int)((k + v - 1) / v);
    return cnt;
  }
};

}  // namespace dwm_sum_avx_internal
