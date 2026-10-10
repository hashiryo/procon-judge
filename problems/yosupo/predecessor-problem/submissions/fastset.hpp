#pragma once
// 競プロでよく見る形の 64 分木の bitset。段 h の i 番目の bit は、段 h - 1 の i 番目の u64 が 0 でないことを表す。
// 段ごとに別の vector を持つ。入れるときと除くときは、毎回いちばん上の段まで書き直す。k 以上の要素は、段を上がり
// ながら k より後ろの bit を探し、見つかった段から ctz で下りる。
#include <vector>
namespace pp_fastset {
using u64= unsigned long long;
struct Set {
 int n, lg;
 std::vector<std::vector<u64>> seg;
 Set(int n, const std::vector<u64>& bits): n(n) {
  seg.push_back(bits);
  for(int m= (n + 63) / 64; m > 1;) {
   const std::vector<u64>& lo= seg.back();
   std::vector<u64> up((m + 63) / 64);
   for(int i= 0; i < m; ++i) up[i / 64]|= u64(lo[i] != 0) << (i % 64);
   seg.push_back(std::move(up));
   m= (m + 63) / 64;
  }
  lg= seg.size();
 }
 bool contains(int i) const { return seg[0][i / 64] >> (i % 64) & 1; }
 void insert(int i) {
  for(int h= 0; h < lg; ++h) seg[h][i / 64]|= u64(1) << (i % 64), i/= 64;
 }
 void erase(int i) {
  u64 x= 0;
  for(int h= 0; h < lg; ++h) {
   seg[h][i / 64]&= ~(u64(1) << (i % 64));
   seg[h][i / 64]|= x << (i % 64);
   x= seg[h][i / 64] != 0;
   i/= 64;
  }
 }
 // k 以上で最小の要素。無ければ -1。
 int next(int i) const {
  for(int h= 0; h < lg; ++h) {
   if(i / 64 == int(seg[h].size())) break;
   const u64 d= seg[h][i / 64] >> (i % 64);
   if(!d) {
    i= i / 64 + 1;
    continue;
   }
   i+= __builtin_ctzll(d);
   for(int g= h - 1; g >= 0; --g) i= i * 64 + __builtin_ctzll(seg[g][i]);
   return i;
  }
  return -1;
 }
 // k 以下で最大の要素。無ければ -1。
 int prev(int i) const {
  for(int h= 0; h < lg; ++h) {
   if(i == -1) break;
   const u64 d= seg[h][i / 64] << (63 - i % 64);
   if(!d) {
    i= i / 64 - 1;
    continue;
   }
   i-= __builtin_clzll(d);
   for(int g= h - 1; g >= 0; --g) i= i * 64 + 63 - __builtin_clzll(seg[g][i]);
   return i;
  }
  return -1;
 }
};
}
struct Solver {
 pp_fastset::Set s;
 Solver(int n, const vector<u64>& bits): s(n, bits) {}
 void insert(int k) { s.insert(k); }
 void erase(int k) { s.erase(k); }
 bool contains(int k) const { return s.contains(k); }
 int next(int k) const { return s.next(k); }
 int prev(int k) const { return s.prev(k); }
};
