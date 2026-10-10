#pragma once
// 診断用の diag_os_* が共有する、NeoLibrary の OrderedSet (neo/data_structure/OrderedSet.hpp) を呼ぶ提出。
// lib-neo-ordered-set と lib-neo-ordered-set-counted と同じ中身で、Counted を引数にした。
#include "pj.hpp"
#include "neo/data_structure/OrderedSet.hpp"
template <bool Counted> struct NeoOrderedSetSolver {
 OrderedSet<Counted> s;
 static vector<int> elements(const vector<u64>& bits) {
  vector<int> a;
  for(int w= 0, nw= bits.size(); w < nw; ++w)
   for(u64 x= bits[w]; x; x&= x - 1) a.push_back(w * 64 + __builtin_ctzll(x));
  return a;
 }
 NeoOrderedSetSolver(int, const vector<u64>& bits): s(elements(bits)) {}
 void insert(int k) { s.insert(k); }
 void erase(int k) { s.erase(k); }
 bool contains(int k) const { return s.contains(k); }
 int next(int k) const { return s.next(k).value_or(-1); }
 int prev(int k) const { return s.prev(k).value_or(-1); }
};
