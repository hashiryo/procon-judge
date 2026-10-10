#pragma once
// fs_flat_hp の葉の段とその 1 つ上の段を、組にして並べたもの。1 つ上の段の語 1 個と、それが指す葉の語 64 個を、65 語
// (520 byte) の塊として続けて置く。葉の語と、その上の語が同じ 4 KB のページに入るので、根まで上がっても TLB の外れが
// 増えない。その代わり、1 つ上の段の語は 520 byte おきに散らばって、まとめて cache に載らなくなる。2 段より上は
// fs_flat_hp と同じく根から順に並べる。探し方、入れ方、除き方は fs_flat と同じで、配列は huge page に置く。
#include <cstdint>
#include <vector>
#ifdef __linux__
#include <sys/mman.h>
#endif
namespace pp_fs_cluster_hp {
using u64= unsigned long long;
struct Set {
 int lg, nw[8];
 size_t off[8];  // 段 h >= 2 の先頭。off[0] は葉と 1 つ上の段の塊の先頭
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
 // 段 h の i 番目の語。
 u64& word(int h, int i) {
  if(h == 0) return a[off[0] + size_t(i / 64) * 65 + 1 + i % 64];
  if(h == 1) return a[off[0] + size_t(i) * 65];
  return a[off[h] + i];
 }
 const u64& word(int h, int i) const { return const_cast<Set*>(this)->word(h, i); }
 Set(int n, const std::vector<u64>& bits): lg(0) {
  for(int m= n;; m= (m + 63) / 64) {
   nw[lg++]= (m + 63) / 64;
   if(m <= 64) break;
  }
  if(lg == 1) nw[lg++]= 1;  // 塊を作るため、1 つ上の段を必ず置く
  size_t tot= 0;
  for(int h= lg; h-- > 2;) off[h]= tot, tot+= nw[h];
  off[0]= tot, tot+= size_t(nw[1]) * 65;
  alloc(tot);
  for(int i= 0; i < nw[0]; ++i) word(0, i)= bits[i];
  for(int h= 1; h < lg; ++h)
   for(int i= 0; i < nw[h - 1]; ++i) word(h, i / 64)|= u64(word(h - 1, i) != 0) << (i % 64);
 }
 bool contains(int i) const { return word(0, i / 64) >> (i % 64) & 1; }
 void insert(int i) {
  for(int h= 0; h < lg; ++h, i/= 64) {
   u64& w= word(h, i / 64);
   const u64 was= w;
   w|= u64(1) << (i % 64);
   if(was) return;
  }
 }
 void erase(int i) {
  for(int h= 0; h < lg; ++h, i/= 64) {
   u64& w= word(h, i / 64);
   w&= ~(u64(1) << (i % 64));
   if(w) return;
  }
 }
 // k 以上で最小の要素。無ければ -1。
 int next(int i) const {
  for(int h= 0; h < lg; ++h) {
   if(i / 64 == nw[h]) break;
   const u64 d= word(h, i / 64) >> (i % 64);
   if(!d) {
    i= i / 64 + 1;
    continue;
   }
   i+= __builtin_ctzll(d);
   for(int g= h - 1; g >= 0; --g) i= i * 64 + __builtin_ctzll(word(g, i));
   return i;
  }
  return -1;
 }
 // k 以下で最大の要素。無ければ -1。
 int prev(int i) const {
  for(int h= 0; h < lg; ++h) {
   if(i == -1) break;
   const u64 d= word(h, i / 64) << (63 - i % 64);
   if(!d) {
    i= i / 64 - 1;
    continue;
   }
   i-= __builtin_clzll(d);
   for(int g= h - 1; g >= 0; --g) i= i * 64 + 63 - __builtin_clzll(word(g, i));
   return i;
  }
  return -1;
 }
};
}
struct Solver {
 pp_fs_cluster_hp::Set s;
 Solver(int n, const vector<u64>& bits): s(n, bits) {}
 void insert(int k) { s.insert(k); }
 void erase(int k) { s.erase(k); }
 bool contains(int k) const { return s.contains(k); }
 int next(int k) const { return s.next(k); }
 int prev(int k) const { return s.prev(k); }
};
