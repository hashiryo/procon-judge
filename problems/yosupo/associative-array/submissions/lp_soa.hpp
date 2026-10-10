#pragma once
// 競プロでよく見る形の線形探索。キー、値、使用中の印 (vector<bool>) を別々の配列に持ち、埋まりが半分を超えたら
// 2 倍にする。ハッシュは lp と同じで、位置は下位 bit で決める。lp とは表の並べ方だけが違い、引くたびに 3 本の
// 配列に触る。
#include <chrono>
#include <vector>
namespace aa_lp_soa {
using u64= unsigned long long;
struct Map {
 std::vector<u64> key, val;
 std::vector<bool> used;
 size_t mask, n, lim;
 u64 seed;
 Map(): key(16), val(16), used(16), mask(15), n(0), lim(8), seed(std::chrono::steady_clock::now().time_since_epoch().count()) {}
 size_t hash(u64 x) const {
  x+= seed;
  x= (x ^ (x >> 30)) * 0xbf58476d1ce4e5b9ull;
  x= (x ^ (x >> 27)) * 0x94d049bb133111ebull;
  return (x ^ (x >> 31)) & mask;
 }
 u64 get(u64 k) const {
  for(size_t i= hash(k);; i= (i + 1) & mask) {
   if(!used[i]) return 0;
   if(key[i] == k) return val[i];
  }
 }
 void set(u64 k, u64 v) {
  size_t i= hash(k);
  for(; used[i]; i= (i + 1) & mask)
   if(key[i] == k) {
    val[i]= v;
    return;
   }
  used[i]= true, key[i]= k, val[i]= v;
  if(++n > lim) grow();
 }
 void grow() {
  std::vector<u64> ok= std::move(key), ov= std::move(val);
  std::vector<bool> ou= std::move(used);
  const size_t nc= ok.size() * 2;
  key.assign(nc, 0), val.assign(nc, 0), used.assign(nc, false);
  mask= nc - 1, lim= nc / 2;
  for(size_t j= 0; j < ok.size(); ++j)
   if(ou[j]) {
    size_t i= hash(ok[j]);
    while(used[i]) i= (i + 1) & mask;
    used[i]= true, key[i]= ok[j], val[i]= ov[j];
   }
 }
};
}
struct Solver {
 aa_lp_soa::Map mp;
 void set(u64 k, u64 v) { mp.set(k, v); }
 u64 get(u64 k) const { return mp.get(k); }
};
