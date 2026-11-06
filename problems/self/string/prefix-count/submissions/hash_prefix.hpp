#pragma once
// トライを作らず、接頭辞のハッシュ (法 2^61 - 1 の多項式、基数は実行ごとに乱択) から、その接頭辞を持つ要素の数への表を持つ。
// 表は開番地法で、鍵のハッシュの上位の bit で場所を決め、線形に探す。足すときは要素のすべての接頭辞の数を増やし、count_prefix は
// p のハッシュを 1 回引くだけ。空の接頭辞は表に入れず、要素の総数を別に持つ。
#include "pj.hpp"
#include <random>

struct Solver {
  static constexpr u64 M = (1ull << 61) - 1;
  static u64 mul(u64 a, u64 b) {
    const unsigned __int128 t = (unsigned __int128)a * b;
    const u64 r = ((u64)t & M) + (u64)(t >> 61);
    return r >= M ? r - M : r;
  }
  u64 B;
  int lg = 16, total = 0;
  size_t used = 0;
  vector<u64> key;  // 0 は空き (ハッシュは 1 以上にずらして入れる)
  vector<int> val;
  Solver() : B((random_device{}() * 0x9E3779B97F4A7C15ull) % (M - 512) + 256), key(size_t(1) << lg, 0), val(size_t(1) << lg, 0) {}
  size_t slot(u64 h) const { return (size_t)((h * 0x9E3779B97F4A7C15ull) >> (64 - lg)); }
  void grow() {
    vector<u64> k2(key.size() * 2, 0);
    vector<int> v2(val.size() * 2, 0);
    ++lg;
    swap(key, k2), swap(val, v2);
    for (size_t i = 0; i < k2.size(); ++i)
      if (k2[i]) {
        size_t j = slot(k2[i]);
        while (key[j]) j = (j + 1) & (key.size() - 1);
        key[j] = k2[i], val[j] = v2[i];
      }
  }
  void add(const string &s, int d) {
    total += d;
    u64 h = 0;
    for (unsigned char c : s) {
      const u64 x = mul(h, B) + c;
      h = x >= M ? x - M : x;
      const u64 k = h + 1;
      size_t j = slot(k);
      while (key[j] && key[j] != k) j = (j + 1) & (key.size() - 1);
      if (!key[j]) {
        if (2 * (used + 1) > key.size()) {
          grow();
          j = slot(k);
          while (key[j]) j = (j + 1) & (key.size() - 1);
        }
        key[j] = k, ++used;
      }
      val[j] += d;
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
    for (size_t j = slot(k);; j = (j + 1) & (key.size() - 1)) {
      if (key[j] == k) return val[j];
      if (!key[j]) return 0;
    }
  }
};
