#pragma once
// 比べるための基準。__gnu_pbds::gp_hash_table (開番地法、表は 2 の冪で下位 bit を使う)。ハッシュは std_umap_sm と
// 同じく、乱数の種を足してから splitmix64 で混ぜる。
#include <chrono>
#include <ext/pb_ds/assoc_container.hpp>
namespace aa_pbds_gp {
using u64= unsigned long long;
struct Hash {
 u64 seed= std::chrono::steady_clock::now().time_since_epoch().count();
 size_t operator()(u64 x) const {
  x+= seed;
  x= (x ^ (x >> 30)) * 0xbf58476d1ce4e5b9ull;
  x= (x ^ (x >> 27)) * 0x94d049bb133111ebull;
  return x ^ (x >> 31);
 }
};
}
struct Solver {
 __gnu_pbds::gp_hash_table<u64, u64, aa_pbds_gp::Hash> mp;
 void set(u64 k, u64 v) { mp[k]= v; }
 u64 get(u64 k) const {
  auto it= mp.find(k);
  return it == mp.end() ? 0 : it->second;
 }
};
