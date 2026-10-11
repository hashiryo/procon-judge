#pragma once
// 期待出力を作る参照実装 (提出としても測る)。窓が左から順に 1 つずつずれることを使い、小さい k 個を入れる multiset
// と残りを入れる multiset を呼び出しをまたいで持つ (想定解の形)。wavelet matrix も永続セグメント木も使わない。
// brute.hpp と小さい入力で突き合わせてある (pj testdata crosscheck --brute brute.hpp)。前の窓 [pl, pr) から [l, r) へ、出る値を除き入る値を入れてから、
// 小さい側の大きさを k に直す。窓が順にずれない呼び出しでは、全部を入れ直す。
#include <set>
#include "pj.hpp"
struct Solver {
 vector<int> a;
 multiset<int> low, high;
 i64 sum= 0;
 int pl= 0, pr= 0;
 explicit Solver(const vector<int>& a_): a(a_) {}
 void erase(int v) {
  auto it= low.find(v);
  if(it != low.end()) sum-= v, low.erase(it);
  else high.erase(high.find(v));
 }
 void fix(int k) {
  while((int)low.size() > k) {
   auto it= prev(low.end());
   sum-= *it, high.insert(*it), low.erase(it);
  }
  while((int)low.size() < k && !high.empty()) {
   auto it= high.begin();
   sum+= *it, low.insert(*it), high.erase(it);
  }
  while(!low.empty() && !high.empty() && *prev(low.end()) > *high.begin()) {
   auto x= prev(low.end()), y= high.begin();
   const int u= *x, v= *y;
   low.erase(x), high.erase(y), low.insert(v), high.insert(u), sum+= v - u;
  }
 }
 i64 sum_smallest(int l, int r, int k) {
  if(l < pl || l > pr || r < pr) {
   low.clear(), high.clear(), sum= 0, pl= pr= l;
  }
  while(pl < l) erase(a[pl++]);
  while(pr < r) low.insert(a[pr]), sum+= a[pr++];
  fix(k);
  return sum;
 }
};
