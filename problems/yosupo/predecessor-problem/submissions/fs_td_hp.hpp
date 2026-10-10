#pragma once
// fs_td の配列を fs_flat_hp と同じく huge page に置くもの。1 回目で効いた 2 つの手を重ねる。段の並べ方と探し方は fs_td と同じ。
#include <cstdint>
#include <vector>
#ifdef __linux__
#include <sys/mman.h>
#endif
namespace pp_fs_td_hp {
using u64= unsigned long long;
struct Set {
 int lg, nw[8];
 size_t off[8];  // 段 h (0 が葉) の先頭
 u64* a;
 size_t bytes;
 bool mapped;
 Set(const Set&)= delete;
 Set& operator=(const Set&)= delete;
 ~Set() {
#ifdef __linux__
  if(mapped) {
   munmap(a, bytes);
   return;
  }
#endif
  delete[] a;
 }
 // 0 で埋まった配列を取る。
 void alloc(size_t words) {
  bytes= words * sizeof(u64), mapped= false;
#ifdef __linux__
  constexpr size_t H= size_t(1) << 21;
  if(bytes >= (size_t(1) << 16)) {
   bytes= (bytes + H - 1) & ~(H - 1);
   char* p= static_cast<char*>(mmap(nullptr, bytes + H, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0));
   char* q= reinterpret_cast<char*>((reinterpret_cast<uintptr_t>(p) + H - 1) & ~(H - 1));
   if(q != p) munmap(p, q - p);
   munmap(q + bytes, p + H - q);
   madvise(q, bytes, MADV_HUGEPAGE);
   a= reinterpret_cast<u64*>(q), mapped= true;
   return;
  }
#endif
  a= new u64[words]();
 }
 Set(int n, const std::vector<u64>& bits): lg(0) {
  for(int m= n;; m= (m + 63) / 64) {
   nw[lg++]= (m + 63) / 64;
   if(m <= 64) break;
  }
  size_t tot= 0;
  for(int h= lg; h--;) off[h]= tot, tot+= nw[h];
  alloc(tot);
  u64* leaf= a + off[0];
  for(int i= 0; i < nw[0]; ++i) leaf[i]= bits[i];
  for(int h= 1; h < lg; ++h) {
   const u64* lo= a + off[h - 1];
   u64* up= a + off[h];
   for(int i= 0; i < nw[h - 1]; ++i) up[i / 64]|= u64(lo[i] != 0) << (i % 64);
  }
 }
 // 葉の語 w が 0 でないか。
 bool live(int w) const { return lg == 1 || (a[off[1] + w / 64] >> (w % 64) & 1); }
 bool contains(int i) const { return live(i / 64) && (a[off[0] + i / 64] >> (i % 64) & 1); }
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
  if(live(i / 64))
   if(const u64 d= a[off[0] + i / 64] >> (i % 64)) return i + __builtin_ctzll(d);
  i= i / 64 + 1;
  for(int h= 1; h < lg; ++h) {
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
  if(live(i / 64))
   if(const u64 d= a[off[0] + i / 64] << (63 - i % 64)) return i - __builtin_clzll(d);
  i= i / 64 - 1;
  for(int h= 1; h < lg; ++h) {
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
 pp_fs_td_hp::Set s;
 Solver(int n, const vector<u64>& bits): s(n, bits) {}
 void insert(int k) { s.insert(k); }
 void erase(int k) { s.erase(k); }
 bool contains(int k) const { return s.contains(k); }
 int next(int k) const { return s.next(k); }
 int prev(int k) const { return s.prev(k); }
};
