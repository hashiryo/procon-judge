#pragma once
// 比べるための基準。std::unordered_map を既定のハッシュ (libstdc++ では整数をそのまま返す) で使う。バケットの数が
// 素数の表から決まるので、その素数の倍数だけを並べた入力 (unordered_map_killer) では全部が 1 つのバケットに入る。
#include <unordered_map>
struct Solver {
 std::unordered_map<u64, u64> mp;
 void set(u64 k, u64 v) { mp[k]= v; }
 u64 get(u64 k) const {
  auto it= mp.find(k);
  return it == mp.end() ? 0 : it->second;
 }
};
