#pragma once
// 開番地法の線形探索。表は {キー, 値} の 16 byte の組を 2 の冪の個数だけ並べたもので、埋まりが半分を超えたら 2 倍に
// する。キーは bit を反転して持ち、0 を空きの印にする。calloc の 0 がそのまま空きになるので、表を埋める手間が要らない。
// 反転すると 0 になるキー (2^64 - 1) だけは、表の外の 1 か所に置く。位置はハッシュの上位 bit で決める。ハッシュは、
// 表を作るときに steady_clock から取った乱数の種を足してから、splitmix64 で混ぜる。
#include <chrono>
#include <cstdlib>
namespace aa_lp {
using u64= unsigned long long;
struct Map {
 struct Slot {
  u64 k, v;
 };
 Slot* t;
 int shift;
 size_t mask, n, lim;
 u64 seed, top;  // top はキー 2^64 - 1 の値
 Map(): t(static_cast<Slot*>(std::calloc(16, sizeof(Slot)))), shift(60), mask(15), n(0), lim(8), seed(std::chrono::steady_clock::now().time_since_epoch().count()), top(0) {}
 ~Map() { std::free(t); }
 Map(const Map&)= delete;
 Map& operator=(const Map&)= delete;
 u64 hash(u64 x) const {
  x+= seed;
  x= (x ^ (x >> 30)) * 0xbf58476d1ce4e5b9ull;
  x= (x ^ (x >> 27)) * 0x94d049bb133111ebull;
  return x ^ (x >> 31);
 }
 u64 get(u64 k) const {
  const u64 c= ~k;
  if(c == 0) [[unlikely]]
   return top;
  for(size_t i= hash(k) >> shift;; i= (i + 1) & mask) {
   if(t[i].k == c) return t[i].v;
   if(t[i].k == 0) return 0;
  }
 }
 void set(u64 k, u64 v) {
  const u64 c= ~k;
  if(c == 0) [[unlikely]] {
   top= v;
   return;
  }
  size_t i= hash(k) >> shift;
  for(; t[i].k != 0; i= (i + 1) & mask)
   if(t[i].k == c) {
    t[i].v= v;
    return;
   }
  t[i]= {c, v};
  if(++n > lim) grow();
 }
 void grow() {
  Slot* old= t;
  const size_t oc= mask + 1, nc= oc * 2;
  t= static_cast<Slot*>(std::calloc(nc, sizeof(Slot)));
  mask= nc - 1, --shift, lim= nc / 2;
  for(size_t j= 0; j < oc; ++j)
   if(old[j].k) {
    size_t i= hash(~old[j].k) >> shift;
    while(t[i].k) i= (i + 1) & mask;
    t[i]= old[j];
   }
  std::free(old);
 }
};
}
struct Solver {
 aa_lp::Map mp;
 void set(u64 k, u64 v) { mp.set(k, v); }
 u64 get(u64 k) const { return mp.get(k); }
};
