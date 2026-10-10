#pragma once
// lp のハッシュを、乱数の奇数を掛けて上位 bit を取るだけの形 (multiply-shift) にしたもの。掛ける数は、表を作るときに
// steady_clock の値を splitmix64 で混ぜて決める。掛け算 1 回で済み、どの 2 つのキーも 2 / (表の大きさ) 以下の確率
// でしか同じ位置にならない。表の形は lp と同じ。
#include <chrono>
#include <cstdlib>
namespace aa_lp_ms {
using u64= unsigned long long;
inline u64 splitmix64(u64 x) {
 x+= 0x9e3779b97f4a7c15ull;
 x= (x ^ (x >> 30)) * 0xbf58476d1ce4e5b9ull;
 x= (x ^ (x >> 27)) * 0x94d049bb133111ebull;
 return x ^ (x >> 31);
}
struct Map {
 struct Slot {
  u64 k, v;
 };
 Slot* t;
 int shift;
 size_t mask, n, lim;
 u64 mul, top;  // top はキー 2^64 - 1 の値
 Map(): t(static_cast<Slot*>(std::calloc(16, sizeof(Slot)))), shift(60), mask(15), n(0), lim(8), mul(splitmix64(std::chrono::steady_clock::now().time_since_epoch().count()) | 1), top(0) {}
 ~Map() { std::free(t); }
 Map(const Map&)= delete;
 Map& operator=(const Map&)= delete;
 u64 get(u64 k) const {
  const u64 c= ~k;
  if(c == 0) [[unlikely]]
   return top;
  for(size_t i= (k * mul) >> shift;; i= (i + 1) & mask) {
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
  size_t i= (k * mul) >> shift;
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
    size_t i= (~old[j].k * mul) >> shift;
    while(t[i].k) i= (i + 1) & mask;
    t[i]= old[j];
   }
  std::free(old);
 }
};
}
struct Solver {
 aa_lp_ms::Map mp;
 void set(u64 k, u64 v) { mp.set(k, v); }
 u64 get(u64 k) const { return mp.get(k); }
};
