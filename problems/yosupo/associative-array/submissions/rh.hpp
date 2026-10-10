#pragma once
// Robin Hood の開番地法。表は {キー, 値} の 16 byte の組と、組ごとに 1 byte の「ホームからの距離 + 1」(0 は空き) の
// 2 本に分ける。入れるときは、ホームから遠い組ほど前に置かれるよう、近い組を後ろへ押し出す。引くときは、今の距離より
// 近い組に当たった時点で、無いと決まる。キーを比べるのは、距離が今の距離と同じ (ホームが同じ) 組だけ。埋まりが 7/8 を
// 超えたら 2 倍にする。距離の 1 byte が溢れそうなときも 2 倍にする。ハッシュと位置の取り方は lp と同じ。
#include <chrono>
#include <cstdlib>
#include <utility>
namespace aa_rh {
using u64= unsigned long long;
using u8= unsigned char;
struct Map {
 struct Slot {
  u64 k, v;
 };
 u8* d;
 Slot* t;
 int shift;
 size_t mask, n, lim;
 u64 seed;
 Map(): d(static_cast<u8*>(std::calloc(16, 1))), t(static_cast<Slot*>(std::malloc(16 * sizeof(Slot)))), shift(60), mask(15), n(0), lim(14), seed(std::chrono::steady_clock::now().time_since_epoch().count()) {}
 ~Map() { std::free(d), std::free(t); }
 Map(const Map&)= delete;
 Map& operator=(const Map&)= delete;
 u64 hash(u64 x) const {
  x+= seed;
  x= (x ^ (x >> 30)) * 0xbf58476d1ce4e5b9ull;
  x= (x ^ (x >> 27)) * 0x94d049bb133111ebull;
  return x ^ (x >> 31);
 }
 u64 get(u64 k) const {
  size_t i= hash(k) >> shift;
  for(unsigned dist= 1;; ++dist, i= (i + 1) & mask) {
   const unsigned m= d[i];
   if(m < dist) return 0;
   if(m == dist && t[i].k == k) return t[i].v;
  }
 }
 void set(u64 k, u64 v) {
  size_t i= hash(k) >> shift;
  unsigned dist= 1;
  for(;; ++dist, i= (i + 1) & mask) {
   const unsigned m= d[i];
   if(m < dist) break;
   if(m == dist && t[i].k == k) {
    t[i].v= v;
    return;
   }
  }
  if(n >= lim) {
   grow();
   insert(Slot{k, v});
  } else place(i, dist, Slot{k, v});
  ++n;
 }
 // 無いと分かっているキーを入れる。
 void insert(Slot s) {
  size_t i= hash(s.k) >> shift;
  unsigned dist= 1;
  for(; d[i] >= dist; ++dist) i= (i + 1) & mask;
  place(i, dist, s);
 }
 // i に距離 dist で s を置き、そこにいた組を後ろへ押し出していく。
 void place(size_t i, unsigned dist, Slot s) {
  for(;; ++dist, i= (i + 1) & mask) {
   if(dist == 255) [[unlikely]] {
    grow();
    insert(s);
    return;
   }
   const unsigned m= d[i];
   if(m == 0) {
    d[i]= dist, t[i]= s;
    return;
   }
   if(m < dist) {
    std::swap(t[i], s);
    d[i]= dist, dist= m;
   }
  }
 }
 void grow() {
  u8* od= d;
  Slot* ot= t;
  const size_t oc= mask + 1, nc= oc * 2;
  d= static_cast<u8*>(std::calloc(nc, 1));
  t= static_cast<Slot*>(std::malloc(nc * sizeof(Slot)));
  mask= nc - 1, --shift, lim= nc / 8 * 7;
  for(size_t j= 0; j < oc; ++j)
   if(od[j]) insert(ot[j]);
  std::free(od), std::free(ot);
 }
};
}
struct Solver {
 aa_rh::Map mp;
 void set(u64 k, u64 v) { mp.set(k, v); }
 u64 get(u64 k) const { return mp.get(k); }
};
