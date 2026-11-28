#pragma once
// 動的 wavelet matrix の核。各段の 0/1 の列を動的ビット列にする。値は 0 以上 2^L 未満の順位として扱い、
// 列への位置指定の挿入・削除・値の変更と、区間の k 番目・x 未満の個数を返す。
//
// 動的ビット列は 2 通り。
//   TreapBitVector: 64 bit または 256 bit の塊を値に持つ treap。節は部分木の bit 数 cnt と 1 の数 ones を持つ。
//   BlockBitVector: 256 bit の塊を並べ、塊の列の上に Fenwick (bit 数と 1 の数) を載せる。塊が満杯になったら
//     128 bit ずつに割り、空になったら外す。Fenwick の下りで塊を探すので、木を下りるより浅い。
// どちらも insert/erase/access は rank を同じ下りで取る (insert_rank/erase_rank/access_rank)。
// 設計と試作の記録は algo-notes の notes/dynamic-wavelet-matrix-problems.md。
#include <cassert>
#include <cstdint>
#include <utility>
#include <vector>

namespace dwm_internal {
using u32 = unsigned;
using u64 = unsigned long long;

inline u64 low_mask(int k) {
  if (k <= 0) return 0;
  if (k >= 64) return ~u64(0);
  return (u64(1) << k) - 1;
}

struct Rng {
  u64 x = 0x9E3779B97F4A7C15ull;
  u64 next() {
    x += 0x9E3779B97F4A7C15ull;
    u64 z = x;
    z = (z ^ (z >> 30)) * 0xBF58476D1CE4E5B9ull;
    z = (z ^ (z >> 27)) * 0x94D049BB133111EBull;
    return z ^ (z >> 31);
  }
};

// 64 bit の塊を持つ treap の動的ビット列。
struct TreapBitVector {
  struct Node {
    u64 val;
    int len, cnt, ones;
    u32 l, r;
    u64 prio;
  };

  static constexpr int CAP = 64;
  std::vector<Node> p;
  Rng rng;
  u32 root = 0;
  int n = 0;

  TreapBitVector() { p.push_back({0, 0, 0, 0, 0, 0, 0}); p.reserve(1 << 16); }

  int size() const { return n; }
  u32 make(u64 val, int len) {
    p.push_back({val, len, len, __builtin_popcountll(val & low_mask(len)), 0, 0, rng.next()});
    return (u32)p.size() - 1;
  }
  int cnt(u32 t) const { return t ? p[t].cnt : 0; }
  int ones(u32 t) const { return t ? p[t].ones : 0; }
  void pull(u32 t) {
    p[t].cnt = cnt(p[t].l) + p[t].len + cnt(p[t].r);
    p[t].ones = ones(p[t].l) + __builtin_popcountll(p[t].val & low_mask(p[t].len)) + ones(p[t].r);
  }
  u32 merge(u32 a, u32 b) {
    if (!a) return b;
    if (!b) return a;
    if (p[a].prio > p[b].prio) {
      p[a].r = merge(p[a].r, b);
      pull(a);
      return a;
    }
    p[b].l = merge(a, p[b].l);
    pull(b);
    return b;
  }
  void insert_bit(u32 t, int pos, bool b) {
    u64 &v = p[t].val;
    u64 lo = v & low_mask(pos);
    u64 hi = v >> pos;
    v = lo | (u64(b) << pos) | (pos + 1 < 64 ? hi << (pos + 1) : 0);
    ++p[t].len;
  }
  void remove_bit(u32 t, int pos) {
    u64 &v = p[t].val;
    int len = p[t].len;
    u64 lo = v & low_mask(pos);
    u64 hi = pos + 1 < 64 ? ((v >> (pos + 1)) & low_mask(len - pos - 1)) : 0;
    v = lo | (hi << pos);
    --p[t].len;
  }

  int rank1(int k) const {
    u32 t = root;
    int res = 0;
    while (t) {
      int lc = cnt(p[t].l);
      if (k < lc) {
        t = p[t].l;
        continue;
      }
      res += ones(p[t].l);
      k -= lc;
      if (k < p[t].len) {
        res += __builtin_popcountll(p[t].val & low_mask(k));
        break;
      }
      res += __builtin_popcountll(p[t].val & low_mask(p[t].len));
      k -= p[t].len;
      t = p[t].r;
    }
    return res;
  }

  std::pair<bool, int> access_rank(int k) const {
    u32 t = root;
    int o = 0;
    while (t) {
      int lc = cnt(p[t].l);
      if (k < lc) {
        t = p[t].l;
        continue;
      }
      o += ones(p[t].l);
      k -= lc;
      if (k < p[t].len) return {(p[t].val >> k) & 1, o + __builtin_popcountll(p[t].val & low_mask(k))};
      o += __builtin_popcountll(p[t].val & low_mask(p[t].len));
      k -= p[t].len;
      t = p[t].r;
    }
    assert(false);
    return {false, 0};
  }

  bool access(int k) const { return access_rank(k).first; }

  std::pair<u32, int> insert_rank_rec(u32 t, int k, bool b) {
    if (!t) return {make(u64(b), 1), 0};
    int lc = cnt(p[t].l);
    if (k < lc) {
      auto [nr, o] = insert_rank_rec(p[t].l, k, b);
      p[t].l = nr;
      pull(t);
      return {t, o};
    }
    if (k > lc + p[t].len) {
      auto [nr, o] = insert_rank_rec(p[t].r, k - lc - p[t].len, b);
      p[t].r = nr;
      pull(t);
      return {t, o + ones(p[t].l) + __builtin_popcountll(p[t].val & low_mask(p[t].len))};
    }
    int pos = k - lc;
    int o = ones(p[t].l) + __builtin_popcountll(p[t].val & low_mask(pos));
    if (p[t].len < CAP) {
      insert_bit(t, pos, b);
      pull(t);
      return {t, o};
    }
    u64 v = p[t].val;
    u64 lo = v & low_mask(32), hi = v >> 32;
    u32 a, c;
    if (pos <= 32) {
      u64 nv = (lo & low_mask(pos)) | (u64(b) << pos) | ((pos < 32 ? (lo >> pos) : u64(0)) << (pos + 1));
      a = make(nv, 33);
      c = make(hi, 32);
    } else {
      int q = pos - 32;
      u64 nv = (hi & low_mask(q)) | (u64(b) << q) | ((q < 32 ? (hi >> q) : u64(0)) << (q + 1));
      a = make(lo, 32);
      c = make(nv, 33);
    }
    u32 L = p[t].l, R = p[t].r;
    return {merge(merge(merge(L, a), c), R), o};
  }

  int insert_rank(int k, bool b) {
    auto [nr, o] = insert_rank_rec(root, k, b);
    root = nr;
    ++n;
    return o;
  }

  struct EraseRes {
    u32 root;
    bool bit;
    int ones_before;
  };
  EraseRes erase_rank_rec(u32 t, int k) {
    int lc = cnt(p[t].l);
    if (k < lc) {
      auto r = erase_rank_rec(p[t].l, k);
      p[t].l = r.root;
      pull(t);
      return {t, r.bit, r.ones_before};
    }
    if (k >= lc + p[t].len) {
      auto r = erase_rank_rec(p[t].r, k - lc - p[t].len);
      p[t].r = r.root;
      pull(t);
      return {t, r.bit, r.ones_before + ones(p[t].l) + __builtin_popcountll(p[t].val & low_mask(p[t].len))};
    }
    int pos = k - lc;
    bool bit = (p[t].val >> pos) & 1;
    int o = ones(p[t].l) + __builtin_popcountll(p[t].val & low_mask(pos));
    remove_bit(t, pos);
    if (p[t].len == 0) return {merge(p[t].l, p[t].r), bit, o};
    pull(t);
    return {t, bit, o};
  }

  std::pair<bool, int> erase_rank(int k) {
    auto r = erase_rank_rec(root, k);
    root = r.root;
    --n;
    return {r.bit, r.ones_before};
  }
};

// 256 bit の塊を持つ treap の動的ビット列。
struct TreapBitVector256 {
  static constexpr int W = 4;
  static constexpr int CAP = 64 * W;
  struct Node {
    u64 w[W];
    int len, cnt, ones;
    u32 l, r;
    u64 prio;
  };

  std::vector<Node> p;
  Rng rng;
  u32 root = 0;
  int n = 0;

  TreapBitVector256() { p.push_back({}); p.reserve(1 << 16); }

  static int chunk_ones(const Node &x) {
    int c = 0;
    for (int i = 0; i < W; ++i) c += __builtin_popcountll(x.w[i] & low_mask(x.len - 64 * i));
    return c;
  }
  static int prefix_ones(const Node &x, int off) {
    int c = 0, i = 0;
    for (; i + 64 <= off; i += 64) c += __builtin_popcountll(x.w[i / 64]);
    if (i < off) c += __builtin_popcountll(x.w[i / 64] & low_mask(off - i));
    return c;
  }
  static bool get_bit(const Node &x, int off) { return (x.w[off >> 6] >> (off & 63)) & 1; }
  static void insert_bit(Node &x, int off, bool b) {
    int word = off >> 6, sh = off & 63;
    for (int i = W - 1; i > word; --i) x.w[i] = (x.w[i] << 1) | (x.w[i - 1] >> 63);
    u64 lo = x.w[word] & low_mask(sh);
    u64 hi = x.w[word] >> sh;
    x.w[word] = lo | (u64(b) << sh) | (sh + 1 < 64 ? hi << (sh + 1) : 0);
    ++x.len;
  }
  static void remove_bit(Node &x, int off) {
    int word = off >> 6, sh = off & 63;
    x.w[word] = (x.w[word] & low_mask(sh)) | ((sh + 1 < 64 ? (x.w[word] >> (sh + 1)) : 0) << sh);
    if (word + 1 < W) x.w[word] |= (x.w[word + 1] & 1) << 63;
    for (int i = word + 1; i < W; ++i) {
      x.w[i] >>= 1;
      if (i + 1 < W) x.w[i] |= (x.w[i + 1] & 1) << 63;
    }
    --x.len;
    x.w[W - 1] &= low_mask(x.len - 64 * (W - 1));
  }

  int size() const { return n; }
  u32 make(const u64 *w, int len) {
    Node x{};
    for (int i = 0; i < W; ++i) x.w[i] = w[i];
    x.len = len;
    x.cnt = len;
    x.ones = chunk_ones(x);
    x.prio = rng.next();
    p.push_back(x);
    return (u32)p.size() - 1;
  }
  u32 make_bit(bool b) {
    u64 w[W] = {u64(b), 0, 0, 0};
    return make(w, 1);
  }
  int cnt(u32 t) const { return t ? p[t].cnt : 0; }
  int ones(u32 t) const { return t ? p[t].ones : 0; }
  void pull(u32 t) {
    p[t].cnt = cnt(p[t].l) + p[t].len + cnt(p[t].r);
    p[t].ones = ones(p[t].l) + chunk_ones(p[t]) + ones(p[t].r);
  }
  u32 merge(u32 a, u32 b) {
    if (!a) return b;
    if (!b) return a;
    if (p[a].prio > p[b].prio) {
      p[a].r = merge(p[a].r, b);
      pull(a);
      return a;
    }
    p[b].l = merge(a, p[b].l);
    pull(b);
    return b;
  }

  int rank1(int k) const {
    u32 t = root;
    int res = 0;
    while (t) {
      int lc = cnt(p[t].l);
      if (k < lc) {
        t = p[t].l;
        continue;
      }
      res += ones(p[t].l);
      k -= lc;
      if (k < p[t].len) {
        res += prefix_ones(p[t], k);
        break;
      }
      res += chunk_ones(p[t]);
      k -= p[t].len;
      t = p[t].r;
    }
    return res;
  }

  std::pair<bool, int> access_rank(int k) const {
    u32 t = root;
    int o = 0;
    while (t) {
      int lc = cnt(p[t].l);
      if (k < lc) {
        t = p[t].l;
        continue;
      }
      o += ones(p[t].l);
      k -= lc;
      if (k < p[t].len) return {get_bit(p[t], k), o + prefix_ones(p[t], k)};
      o += chunk_ones(p[t]);
      k -= p[t].len;
      t = p[t].r;
    }
    assert(false);
    return {false, 0};
  }

  bool access(int k) const { return access_rank(k).first; }

  std::pair<u32, int> insert_rank_rec(u32 t, int k, bool b) {
    if (!t) return {make_bit(b), 0};
    int lc = cnt(p[t].l);
    if (k < lc) {
      auto [nr, o] = insert_rank_rec(p[t].l, k, b);
      p[t].l = nr;
      pull(t);
      return {t, o};
    }
    if (k > lc + p[t].len) {
      auto [nr, o] = insert_rank_rec(p[t].r, k - lc - p[t].len, b);
      p[t].r = nr;
      pull(t);
      return {t, o + ones(p[t].l) + chunk_ones(p[t])};
    }
    int pos = k - lc;
    int o = ones(p[t].l) + prefix_ones(p[t], pos);
    if (p[t].len < CAP) {
      insert_bit(p[t], pos, b);
      pull(t);
      return {t, o};
    }
    u64 lo[W] = {p[t].w[0], p[t].w[1], 0, 0};
    u64 hi[W] = {p[t].w[2], p[t].w[3], 0, 0};
    u32 a, c;
    if (pos <= 128) {
      Node x{};
      for (int i = 0; i < W; ++i) x.w[i] = lo[i];
      x.len = 128;
      insert_bit(x, pos, b);
      a = make(x.w, x.len);
      c = make(hi, 128);
    } else {
      Node x{};
      for (int i = 0; i < W; ++i) x.w[i] = hi[i];
      x.len = 128;
      insert_bit(x, pos - 128, b);
      a = make(lo, 128);
      c = make(x.w, x.len);
    }
    u32 L = p[t].l, R = p[t].r;
    return {merge(merge(merge(L, a), c), R), o};
  }

  int insert_rank(int k, bool b) {
    auto [nr, o] = insert_rank_rec(root, k, b);
    root = nr;
    ++n;
    return o;
  }

  struct EraseRes {
    u32 root;
    bool bit;
    int ones_before;
  };
  EraseRes erase_rank_rec(u32 t, int k) {
    int lc = cnt(p[t].l);
    if (k < lc) {
      auto r = erase_rank_rec(p[t].l, k);
      p[t].l = r.root;
      pull(t);
      return {t, r.bit, r.ones_before};
    }
    if (k >= lc + p[t].len) {
      auto r = erase_rank_rec(p[t].r, k - lc - p[t].len);
      p[t].r = r.root;
      pull(t);
      return {t, r.bit, r.ones_before + ones(p[t].l) + chunk_ones(p[t])};
    }
    int pos = k - lc;
    bool bit = get_bit(p[t], pos);
    int o = ones(p[t].l) + prefix_ones(p[t], pos);
    remove_bit(p[t], pos);
    if (p[t].len == 0) return {merge(p[t].l, p[t].r), bit, o};
    pull(t);
    return {t, bit, o};
  }

  std::pair<bool, int> erase_rank(int k) {
    auto r = erase_rank_rec(root, k);
    root = r.root;
    --n;
    return {r.bit, r.ones_before};
  }
};

// 256 bit の塊を並べ、塊の列の上に Fenwick を載せる動的ビット列。
struct BlockBitVector {
  static constexpr int W = 4;
  static constexpr int CAP = 64 * W;

  struct Block {
    u64 w[W];
    int len;
  };

  std::vector<Block> blk;
  std::vector<int> fcnt, fones;  // 1-indexed Fenwick。f[i] は塊 [i-lowbit(i), i)
  int n = 0;
  int total_ones = 0;

  BlockBitVector() = default;

  int size() const { return n; }

  static int block_ones(const Block &b) {
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

  void rebuild() {
    int m = (int)blk.size();
    fcnt.assign(m + 1, 0);
    fones.assign(m + 1, 0);
    total_ones = 0;
    for (int i = 0; i < m; ++i) {
      fcnt[i + 1] = blk[i].len;
      fones[i + 1] = block_ones(blk[i]);
      total_ones += fones[i + 1];
    }
    for (int i = 1; i <= m; ++i) {
      int j = i + (i & -i);
      if (j <= m) fcnt[j] += fcnt[i], fones[j] += fones[i];
    }
  }

  // k 番目の bit を含む塊と、その塊の中の位置。k == n のときは b == m, off == 0。
  // acc_ones には塊 b より前の 1 の数が入る。
  void find(int k, int &b, int &off, int &acc_ones) const {
    int m = (int)blk.size();
    if (k >= n) {
      b = m;
      off = 0;
      acc_ones = total_ones;
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

  int rank1(int k) const {
    int b, off, acc_ones;
    find(k, b, off, acc_ones);
    if (b == (int)blk.size()) return acc_ones;
    return acc_ones + prefix_ones(blk[b], off);
  }

  std::pair<bool, int> access_rank(int k) const {
    int b, off, acc_ones;
    find(k, b, off, acc_ones);
    assert(b < (int)blk.size());
    return {get_bit(blk[b], off), acc_ones + prefix_ones(blk[b], off)};
  }

  bool access(int k) const { return access_rank(k).first; }

  static void insert_bit(Block &b, int off, bool bit) {
    int word = off >> 6, sh = off & 63;
    for (int i = W - 1; i > word; --i) b.w[i] = (b.w[i] << 1) | (b.w[i - 1] >> 63);
    u64 lo = b.w[word] & low_mask(sh);
    u64 hi = b.w[word] >> sh;
    b.w[word] = lo | (u64(bit) << sh) | (sh + 1 < 64 ? hi << (sh + 1) : 0);
    ++b.len;
  }
  static void remove_bit(Block &b, int off) {
    int word = off >> 6, sh = off & 63;
    b.w[word] = (b.w[word] & low_mask(sh)) | ((sh + 1 < 64 ? (b.w[word] >> (sh + 1)) : 0) << sh);
    if (word + 1 < W) b.w[word] |= (b.w[word + 1] & 1) << 63;
    for (int i = word + 1; i < W; ++i) {
      b.w[i] >>= 1;
      if (i + 1 < W) b.w[i] |= (b.w[i + 1] & 1) << 63;
    }
    --b.len;
    b.w[W - 1] &= low_mask(b.len - 64 * (W - 1));
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
    blk[b] = lo;
    blk.insert(blk.begin() + b + 1, hi);
    rebuild();
  }

  int insert_rank(int k, bool bit) {
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
      int ones_before = total_ones;
      insert_bit(blk[b], blk[b].len, bit);
      ++n;
      total_ones += bit;
      for (int i = b + 1; i <= (int)blk.size(); i += i & -i) fcnt[i] += 1, fones[i] += bit;
      return ones_before;
    }
    int b, off, acc_ones;
    find(k, b, off, acc_ones);
    if (blk[b].len == CAP) {
      split_block(b);
      find(k, b, off, acc_ones);
    }
    int ones_before = acc_ones + prefix_ones(blk[b], off);
    insert_bit(blk[b], off, bit);
    ++n;
    total_ones += bit;
    for (int i = b + 1; i <= (int)blk.size(); i += i & -i) fcnt[i] += 1, fones[i] += bit;
    return ones_before;
  }

  std::pair<bool, int> erase_rank(int k) {
    int b, off, acc_ones;
    find(k, b, off, acc_ones);
    bool bit = get_bit(blk[b], off);
    int ones_before = acc_ones + prefix_ones(blk[b], off);
    remove_bit(blk[b], off);
    --n;
    total_ones -= bit;
    for (int i = b + 1; i <= (int)blk.size(); i += i & -i) fcnt[i] -= 1, fones[i] -= bit;
    if (blk[b].len == 0) {
      blk.erase(blk.begin() + b);
      rebuild();
    } else if (blk[b].len < 64 && b + 1 < (int)blk.size() && blk[b].len + blk[b + 1].len <= CAP) {
      int add = blk[b + 1].len;
      for (int i = 0; i < add; ++i) insert_bit(blk[b], blk[b].len, get_bit(blk[b + 1], i));
      blk.erase(blk.begin() + b + 1);
      rebuild();
      (void)add;
    }
    return {bit, ones_before};
  }
};

// 2 分の動的 wavelet matrix。BV は動的ビット列の型。
template <int L, class BV = TreapBitVector>
struct DynamicWaveletMatrix {
  BV bv[L];
  int mid[L] = {};
  int n = 0;

  DynamicWaveletMatrix() = default;

  template <class V>
  explicit DynamicWaveletMatrix(const std::vector<V> &a) {
    n = (int)a.size();
    std::vector<int> cur(n), nxt(n), zero(n), one(n);
    for (int i = 0; i < n; ++i) cur[i] = (int)a[i];
    for (int h = L - 1; h >= 0; --h) {
      int z = 0, o = 0;
      for (int i = 0; i < n; ++i) {
        bool b = (cur[i] >> h) & 1;
        bv[h].insert_rank(i, b);
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

  void insert(int k, int x) {
    for (int h = L - 1; h >= 0; --h) {
      bool b = (x >> h) & 1;
      int o = bv[h].insert_rank(k, b);
      if (b) k = mid[h] + o;
      else k -= o, ++mid[h];
    }
    ++n;
  }

  // 位置 k を消し、消した値を返す。
  int erase_value(int k) {
    int res = 0;
    for (int h = L - 1; h >= 0; --h) {
      auto [b, o] = bv[h].erase_rank(k);
      if (b) res |= 1 << h, k = mid[h] + o;
      else k -= o, --mid[h];
    }
    --n;
    return res;
  }

  void erase(int k) { (void)erase_value(k); }

  int access(int k) const {
    int res = 0;
    for (int h = L - 1; h >= 0; --h) {
      auto [b, o] = bv[h].access_rank(k);
      if (b) res |= 1 << h, k = mid[h] + o;
      else k -= o;
    }
    return res;
  }

  void set(int k, int x) {
    erase_value(k);
    insert(k, x);
  }

  int kth(int l, int r, int k) const {
    int res = 0;
    for (int h = L - 1; h >= 0; --h) {
      int ol = bv[h].rank1(l), orr = bv[h].rank1(r);
      int z = (r - l) - (orr - ol);
      if (k < z) {
        l -= ol;
        r -= orr;
      } else {
        k -= z;
        res |= 1 << h;
        l = mid[h] + ol;
        r = mid[h] + orr;
      }
    }
    return res;
  }

  int count_lt(int l, int r, int x) const {
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

  // [l, r) の x の個数。
  int count(int l, int r, int x) const {
    for (int h = L - 1; h >= 0; --h) {
      int ol = bv[h].rank1(l), orr = bv[h].rank1(r);
      if ((x >> h) & 1) {
        l = mid[h] + ol;
        r = mid[h] + orr;
      } else {
        l -= ol;
        r -= orr;
      }
    }
    return r - l;
  }
};

}  // namespace dwm_internal
