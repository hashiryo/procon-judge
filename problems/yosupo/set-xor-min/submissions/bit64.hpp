#pragma once
#include "pj.hpp"
#include <bit>
#include <new>

// 64 分岐のビット集合を重ねた木。値は 30 bit で、上から 3、6、6、6 bit の 4 段で 512 個ずつの塊まで下り、
// 塊の中の 9 bit は 512 bit (64 byte) の塊に持つ。塊は 2^21 個あり、塊ごとの表に「空」「値が 1 つだけ (その値の
// 下 9 bit)」「塊の番号」のどれかを持つ。値が散らばると多くの塊は値が 1 つなので、塊を作らずに表だけで済む。
// Library Checker の 112939 (匿名) と同じ作り。
//
// xor_min(x) は、各段で、立っている子の番号 j のうち j xor t (t は x の対応する bit) が最小のものを選んで下りる。
// 64 bit の語 m で j xor t が最小の j は、上の bit から順に、t と同じ側に立っている bit があればそちらへ絞って求める。
struct Solver {
  static constexpr u32 ONE = 1u << 31;  // 表の値が 1 つだけの印
  struct alignas(64) Block {
    u64 w[8];
  };
  u64 m0 = 0, m1[8] = {}, m2[1 << 9] = {};
  vector<u64> m3;
  vector<u32> tab;
  vector<Block> blocks;

  Solver() : m3(1 << 15), tab(1 << 21) { blocks.reserve(1 << 12); }

  static u32 pick(u64 m, u32 t) {
    constexpr u64 K[6] = {0x5555555555555555ULL, 0x3333333333333333ULL, 0x0F0F0F0F0F0F0F0FULL,
                          0x00FF00FF00FF00FFULL, 0x0000FFFF0000FFFFULL, 0x00000000FFFFFFFFULL};
    for (int k = 5; k >= 0; --k) {
      const u64 c = m & (K[k] ^ (0 - u64((t >> k) & 1)));
      m = c ? c : m;
    }
    return u32(countr_zero(m));
  }
  static bool empty(const Block &b) {
    return !(b.w[0] | b.w[1] | b.w[2] | b.w[3] | b.w[4] | b.w[5] | b.w[6] | b.w[7]);
  }

  // 塊 b が空から空でなくなった。
  void up(u32 b) {
    u64 &w3 = m3[b >> 6];
    const bool e3 = !w3;
    w3 |= u64(1) << (b & 63);
    if (!e3) return;
    b >>= 6;
    u64 &w2 = m2[b >> 6];
    const bool e2 = !w2;
    w2 |= u64(1) << (b & 63);
    if (!e2) return;
    b >>= 6;
    u64 &w1 = m1[b >> 6];
    const bool e1 = !w1;
    w1 |= u64(1) << (b & 63);
    if (!e1) return;
    m0 |= u64(1) << (b >> 6);
  }
  // 塊 b が空になった。
  void down(u32 b) {
    if (m3[b >> 6] &= ~(u64(1) << (b & 63))) return;
    b >>= 6;
    if (m2[b >> 6] &= ~(u64(1) << (b & 63))) return;
    b >>= 6;
    if (m1[b >> 6] &= ~(u64(1) << (b & 63))) return;
    m0 &= ~(u64(1) << (b >> 6));
  }

  void insert(int x_) {
    const u32 x = x_, b = x >> 9, lo = x & 511;
    const u32 e = tab[b];
    if (e == 0) {
      tab[b] = ONE | lo;
      up(b);
    } else if (e & ONE) {
      if ((e ^ ONE) == lo) return;
      blocks.push_back({});
      Block &k = blocks.back();
      k.w[lo >> 6] |= u64(1) << (lo & 63);
      k.w[(e >> 6) & 7] |= u64(1) << (e & 63);
      tab[b] = u32(blocks.size());
    } else {
      Block &k = blocks[e - 1];
      const bool was = empty(k);
      k.w[lo >> 6] |= u64(1) << (lo & 63);
      if (was) up(b);
    }
  }

  void erase(int x_) {
    const u32 x = x_, b = x >> 9, lo = x & 511;
    const u32 e = tab[b];
    if (e == 0) return;
    if (e & ONE) {
      if ((e ^ ONE) != lo) return;
      tab[b] = 0;
      down(b);
    } else {
      Block &k = blocks[e - 1];
      u64 &w = k.w[lo >> 6];
      const u64 bit = u64(1) << (lo & 63);
      if (!(w & bit)) return;
      w ^= bit;
      if (empty(k)) down(b);
    }
  }

  int xor_min(int x_) {
    const u32 x = x_;
    u32 b = pick(m0, x >> 27);
    b = b << 6 | pick(m1[b], (x >> 21) & 63);
    b = b << 6 | pick(m2[b], (x >> 15) & 63);
    b = b << 6 | pick(m3[b], (x >> 9) & 63);
    const u32 e = tab[b];
    u32 lo;
    if (e & ONE) lo = e ^ ONE;
    else {
      const Block &k = blocks[e - 1];
      u64 nz = 0;
      for (int i = 0; i < 8; ++i) nz |= u64(k.w[i] != 0) << i;
      const u32 wi = pick(nz, (x >> 6) & 7);
      lo = wi << 6 | pick(k.w[wi], x & 63);
    }
    return int(((b << 9) | lo) ^ x);
  }
};
