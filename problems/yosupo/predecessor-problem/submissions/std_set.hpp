#pragma once
// 比べるための基準。std::set<int> (赤黒木)。初期集合は昇順に末尾へ足すので、構築は O(N)。
#include <iterator>
#include <set>
struct Solver {
 std::set<int> s;
 Solver(int, const vector<u64>& bits) {
  for(int w= 0, nw= bits.size(); w < nw; ++w)
   for(u64 x= bits[w]; x; x&= x - 1) s.emplace_hint(s.end(), w * 64 + __builtin_ctzll(x));
 }
 void insert(int k) { s.insert(k); }
 void erase(int k) { s.erase(k); }
 bool contains(int k) const { return s.count(k); }
 int next(int k) const {
  auto it= s.lower_bound(k);
  return it == s.end() ? -1 : *it;
 }
 int prev(int k) const {
  auto it= s.upper_bound(k);
  return it == s.begin() ? -1 : *std::prev(it);
 }
};
