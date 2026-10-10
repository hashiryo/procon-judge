#pragma once
// NeoLibrary の OrderedSet<true> (neo/data_structure/OrderedSet.hpp)。B+ 木で、k 番目と個数のための子の個数と組の累積を持つ形。
// 個数を持つかどうかで入れる・除くの手間がどれだけ変わるかを、この問題で lib-neo-ordered-set と比べる。
// 初期集合はビットの列から昇順の要素の列を作ってから組む。
#include "pj.hpp"
#include "neo/data_structure/OrderedSet.hpp"
struct Solver {
 OrderedSet<true> s;
 static vector<int> elements(const vector<u64>& bits) {
  vector<int> a;
  for(int w= 0, nw= bits.size(); w < nw; ++w)
   for(u64 x= bits[w]; x; x&= x - 1) a.push_back(w * 64 + __builtin_ctzll(x));
  return a;
 }
 Solver(int, const vector<u64>& bits): s(elements(bits)) {}
 void insert(int k) { s.insert(k); }
 void erase(int k) { s.erase(k); }
 bool contains(int k) const { return s.contains(k); }
 int next(int k) const { return s.next(k).value_or(-1); }
 int prev(int k) const { return s.prev(k).value_or(-1); }
};
