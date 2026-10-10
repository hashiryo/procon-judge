#pragma once
// fastset の段を 1 本の配列に並べたもの。根の段を先頭に置き、葉の段を最後に置く。上の段は合わせても葉の 1/63 ほどで、
// 配列の先頭にまとまる。入れるときは、bit が立っていた語に当たったら上の段はもう正しいので止める。除くときは、語が
// 0 にならなければ止める。探し方は fastset と同じ。
#include <memory>
#include <vector>
namespace pp_fs_flat {
using u64= unsigned long long;
struct Set {
 int lg, nw[8];
 size_t off[8];  // 段 h (0 が葉) の先頭
 std::unique_ptr<u64[]> a;
 Set(int n, const std::vector<u64>& bits): lg(0) {
  for(int m= n;; m= (m + 63) / 64) {
   nw[lg++]= (m + 63) / 64;
   if(m <= 64) break;
  }
  size_t tot= 0;
  for(int h= lg; h--;) off[h]= tot, tot+= nw[h];
  a.reset(new u64[tot]());
  u64* leaf= a.get() + off[0];
  for(int i= 0; i < nw[0]; ++i) leaf[i]= bits[i];
  for(int h= 1; h < lg; ++h) {
   const u64* lo= a.get() + off[h - 1];
   u64* up= a.get() + off[h];
   for(int i= 0; i < nw[h - 1]; ++i) up[i / 64]|= u64(lo[i] != 0) << (i % 64);
  }
 }
 bool contains(int i) const { return a[off[0] + i / 64] >> (i % 64) & 1; }
 void insert(int i) {
  for(int h= 0; h < lg; ++h, i/= 64) {
   u64& w= a[off[h] + i / 64];
   const u64 was= w;
   w|= u64(1) << (i % 64);
   if(was) return;
  }
 }
 void erase(int i) {
  for(int h= 0; h < lg; ++h, i/= 64) {
   u64& w= a[off[h] + i / 64];
   w&= ~(u64(1) << (i % 64));
   if(w) return;
  }
 }
 // k 以上で最小の要素。無ければ -1。
 int next(int i) const {
  for(int h= 0; h < lg; ++h) {
   if(i / 64 == nw[h]) break;
   const u64 d= a[off[h] + i / 64] >> (i % 64);
   if(!d) {
    i= i / 64 + 1;
    continue;
   }
   i+= __builtin_ctzll(d);
   for(int g= h - 1; g >= 0; --g) i= i * 64 + __builtin_ctzll(a[off[g] + i]);
   return i;
  }
  return -1;
 }
 // k 以下で最大の要素。無ければ -1。
 int prev(int i) const {
  for(int h= 0; h < lg; ++h) {
   if(i == -1) break;
   const u64 d= a[off[h] + i / 64] << (63 - i % 64);
   if(!d) {
    i= i / 64 - 1;
    continue;
   }
   i-= __builtin_clzll(d);
   for(int g= h - 1; g >= 0; --g) i= i * 64 + 63 - __builtin_clzll(a[off[g] + i]);
   return i;
  }
  return -1;
 }
};
}
struct Solver {
 pp_fs_flat::Set s;
 Solver(int n, const vector<u64>& bits): s(n, bits) {}
 void insert(int k) { s.insert(k); }
 void erase(int k) { s.erase(k); }
 bool contains(int k) const { return s.contains(k); }
 int next(int k) const { return s.next(k); }
 int prev(int k) const { return s.prev(k); }
};
