#pragma once
// hash_prefix の表を 2 つ直したもの。1 つ目は、鍵 (接頭辞のハッシュ) と数を 16 byte の 1 つの欄にまとめ、1 回の探索の読み込みを
// 1 回にする (別々の配列だと 2 回)。2 つ目は、表を 2 MB 以上なら huge page に置く (_shared/string/_hugealloc.hpp)。追加と削除は
// 要素のすべての接頭辞の数を増減し、count_prefix は p のハッシュを 1 回引く。接頭辞ごとの探索は互いに待たないので、トライのように
// 1 つ前の頂点の読み込みを待たずに済む。
#include "pj.hpp"
#include <random>
#include "_shared/string/_hugealloc.hpp"

struct Solver {
  static constexpr u64 M = (1ull << 61) - 1;
  static u64 mul(u64 a, u64 b) {
    const unsigned __int128 t = (unsigned __int128)a * b;
    const u64 r = ((u64)t & M) + (u64)(t >> 61);
    return r >= M ? r - M : r;
  }
  struct Slot {
    u64 key;  // 0 は空き (ハッシュに 1 を足して入れる)
    long long val;
  };
  u64 B;
  int lg = 12, total = 0;
  size_t used = 0, mask;
  string_hp::Vec<Slot> tab;
  Solver() : B((random_device{}() * 0x9E3779B97F4A7C15ull) % (M - 512) + 256), mask(((size_t)1 << lg) - 1), tab((size_t)1 << lg, Slot{0, 0}) {}
  size_t slot(u64 k) const { return (size_t)((k * 0x9E3779B97F4A7C15ull) >> (64 - lg)); }
  void grow() {
    string_hp::Vec<Slot> old((size_t)2 << lg, Slot{0, 0});
    old.swap(tab);
    ++lg, mask = tab.size() - 1;
    for (const Slot &e : old)
      if (e.key) {
        size_t j = slot(e.key);
        while (tab[j].key) j = (j + 1) & mask;
        tab[j] = e;
      }
  }
  void add(const string &s, int d) {
    total += d;
    if (2 * (used + s.size()) > tab.size())
      while (2 * (used + s.size()) > tab.size()) grow();
    u64 h = 0;
    for (unsigned char c : s) {
      const u64 x = mul(h, B) + c;
      h = x >= M ? x - M : x;
      const u64 k = h + 1;
      size_t j = slot(k);
      while (tab[j].key && tab[j].key != k) j = (j + 1) & mask;
      used += !tab[j].key;
      tab[j].key = k, tab[j].val += d;
    }
  }
  void insert(const string &s) { add(s, 1); }
  void erase(const string &s) { add(s, -1); }
  int count_prefix(const string &p) {
    if (p.empty()) return total;
    u64 h = 0;
    for (unsigned char c : p) {
      const u64 x = mul(h, B) + c;
      h = x >= M ? x - M : x;
    }
    const u64 k = h + 1;
    for (size_t j = slot(k);; j = (j + 1) & mask) {
      if (tab[j].key == k) return (int)tab[j].val;
      if (!tab[j].key) return 0;
    }
  }
};
