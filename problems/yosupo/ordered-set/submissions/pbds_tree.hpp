#pragma once
// 比べるための基準。__gnu_pbds::tree (赤黒木) に部分木の大きさを持たせる tree_order_statistics_node_update を付けたもの。
// k 番目は find_by_order、x 以下の個数は order_of_key(x + 1) で引く。初期集合は 1 つずつ入れる。
#include <ext/pb_ds/assoc_container.hpp>
#include <ext/pb_ds/tree_policy.hpp>
#include <functional>
struct Solver {
 __gnu_pbds::tree<int, __gnu_pbds::null_type, std::less<int>, __gnu_pbds::rb_tree_tag, __gnu_pbds::tree_order_statistics_node_update> s;
 explicit Solver(const vector<int>& a) {
  for(int x: a) s.insert(x);
 }
 void insert(int x) { s.insert(x); }
 void erase(int x) { s.erase(x); }
 int kth(int k) const {
  if(k >= int(s.size())) return -1;
  return *s.find_by_order(k);
 }
 int count_le(int x) const { return s.order_of_key(x + 1); }
 int prev(int x) const {
  auto it= s.upper_bound(x);
  return it == s.begin() ? -1 : *--it;
 }
 int next(int x) const {
  auto it= s.lower_bound(x);
  return it == s.end() ? -1 : *it;
 }
};
