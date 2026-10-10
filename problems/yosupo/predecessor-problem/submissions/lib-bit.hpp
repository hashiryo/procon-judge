#pragma once
// Library の BinaryIndexedTree (mylib/data_structure/BinaryIndexedTree.hpp) で要素の個数を数える。k 以上で最小の要素は
// 「k 未満の個数」番目の要素で、find (累積和が初めてその値を超える位置) で引く。入っているかは別の 1 byte の配列で見る。
#include "mylib/data_structure/BinaryIndexedTree.hpp"
struct Solver {
 int n, total;
 std::vector<char> in;
 BinaryIndexedTree<int> bit;
 static std::vector<int> unpack(int n, const vector<u64>& bits) {
  std::vector<int> a(n);
  for(int i= 0; i < n; ++i) a[i]= bits[i >> 6] >> (i & 63) & 1;
  return a;
 }
 Solver(int n, const vector<u64>& bits): n(n), total(0), in(n), bit(unpack(n, bits)) {
  for(int i= 0; i < n; ++i) total+= in[i]= bits[i >> 6] >> (i & 63) & 1;
 }
 void insert(int k) {
  if(!in[k]) in[k]= 1, ++total, bit.add(k, 1);
 }
 void erase(int k) {
  if(in[k]) in[k]= 0, --total, bit.add(k, -1);
 }
 bool contains(int k) const { return in[k]; }
 int next(int k) const {
  const int c= bit.sum(k);
  return c == total ? -1 : bit.find(c);
 }
 int prev(int k) const {
  const int c= bit.sum(k + 1);
  return c == 0 ? -1 : bit.find(c - 1);
 }
};
