#pragma once
#include "pj.hpp"

// 教科書どおりの二分トライ。節は配列に置いて子を添字で持ち、部分木の値の数を持つ。xor_min(x) は根から 30 段、
// x の bit と同じ側の子に値があればそちらへ、無ければ反対へ下りる。
struct Solver {
  static constexpr int B = 30;
  vector<array<u32, 2>> ch;
  vector<u32> cnt;

  Solver() {
    ch.reserve(1 << 16), cnt.reserve(1 << 16);
    ch.push_back({0, 0}), cnt.push_back(0);  // 根
  }

  void insert(int x) {
    u32 path[B + 1];
    u32 v = 0;
    for (int k = B - 1; k >= 0; --k) {
      path[k + 1] = v;
      const int b = (x >> k) & 1;
      if (!ch[v][b]) {
        ch[v][b] = u32(ch.size());
        ch.push_back({0, 0}), cnt.push_back(0);
      }
      v = ch[v][b];
    }
    if (cnt[v]) return;
    cnt[v] = 1;
    for (int k = 1; k <= B; ++k) ++cnt[path[k]];
  }

  void erase(int x) {
    u32 path[B + 1];
    u32 v = 0;
    for (int k = B - 1; k >= 0; --k) {
      path[k + 1] = v;
      v = ch[v][(x >> k) & 1];
      if (!v) return;
    }
    if (!cnt[v]) return;
    cnt[v] = 0;
    for (int k = 1; k <= B; ++k) --cnt[path[k]];
  }

  int xor_min(int x) {
    u32 v = 0;
    int res = 0;
    for (int k = B - 1; k >= 0; --k) {
      const int b = (x >> k) & 1;
      const u32 c = ch[v][b];
      if (c && cnt[c]) v = c;
      else v = ch[v][b ^ 1], res |= 1 << k;
    }
    return res;
  }
};
