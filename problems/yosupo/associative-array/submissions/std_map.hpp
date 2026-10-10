#pragma once
// 比べるための基準。順序を持つ std::map (赤黒木)。1 回の操作で O(log N) 個の節点をたどる。
#include <map>
struct Solver {
 std::map<u64, u64> mp;
 void set(u64 k, u64 v) { mp[k]= v; }
 u64 get(u64 k) const {
  auto it= mp.find(k);
  return it == mp.end() ? 0 : it->second;
 }
};
