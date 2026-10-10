#pragma once
// 比べるための基準。std::unordered_map に、乱数の種を足してから splitmix64 で混ぜるハッシュを渡す。種は表を作る
// ときに steady_clock から取る。
#include <chrono>
#include <unordered_map>
namespace aa_std_umap_sm {
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
 std::unordered_map<u64, u64, aa_std_umap_sm::Hash> mp;
 void set(u64 k, u64 v) { mp[k]= v; }
 u64 get(u64 k) const {
  auto it= mp.find(k);
  return it == mp.end() ? 0 : it->second;
 }
};
