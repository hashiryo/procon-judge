#pragma once
// 昇順の列を塊に分けて持つ、高さ 2 の B 木のようなもの (Python の SortedList と同じ形)。塊は最初 512 個ずつにし、
// 入れて 1024 個になったら半分に割り、除いて空になったら外す。上の段には、塊ごとの最大と、塊の大きさの Fenwick tree を
// 持つ。キーで引くときは、最大の列を二分探索して塊を選び、塊の中を二分探索する。k 番目は Fenwick tree を下りて塊と
// 位置を決める。塊を割るときと外すときは上の段を作り直す (塊の数に比例する手間で、割るのは 512 回に 1 回ほど)。
#include <algorithm>
#include <bit>
#include <vector>
namespace os_blocks {
struct Set {
 static constexpr int B= 512;
 std::vector<std::vector<int>> blk;
 std::vector<int> mx, fw;  // 塊ごとの最大、塊の大きさの Fenwick tree (1 始まり)
 int total;
 explicit Set(const std::vector<int>& a): total(a.size()) {
  for(size_t i= 0; i < a.size(); i+= B) blk.emplace_back(a.begin() + i, a.begin() + std::min(a.size(), i + B));
  rebuild();
 }
 void rebuild() {
  const int m= blk.size();
  mx.resize(m), fw.assign(m + 1, 0);
  for(int j= 0; j < m; ++j) mx[j]= blk[j].back(), fw[j + 1]= blk[j].size();
  for(int j= 1; j <= m; ++j)
   if(const int p= j + (j & -j); p <= m) fw[p]+= fw[j];
 }
 void add(int j, int v) {
  for(++j; j < int(fw.size()); j+= j & -j) fw[j]+= v;
 }
 // 塊 [0, j) の要素の個数。
 int prefix(int j) const {
  int s= 0;
  for(; j; j&= j - 1) s+= fw[j];
  return s;
 }
 // 最大が k 以上の最初の塊。無ければ塊の数。
 int find(int k) const { return std::lower_bound(mx.begin(), mx.end(), k) - mx.begin(); }
 void insert(int k) {
  if(blk.empty()) {
   blk.push_back({k}), total= 1;
   return rebuild();
  }
  int j= find(k);
  if(j == int(blk.size())) --j;
  std::vector<int>& b= blk[j];
  const auto it= std::lower_bound(b.begin(), b.end(), k);
  if(it != b.end() && *it == k) return;
  b.insert(it, k), ++total;
  if(int(b.size()) == 2 * B) {
   blk.emplace(blk.begin() + j + 1, blk[j].begin() + B, blk[j].end());
   blk[j].resize(B);
   return rebuild();
  }
  mx[j]= b.back(), add(j, 1);
 }
 void erase(int k) {
  const int j= find(k);
  if(j == int(blk.size())) return;
  std::vector<int>& b= blk[j];
  const auto it= std::lower_bound(b.begin(), b.end(), k);
  if(*it != k) return;
  b.erase(it), --total;
  if(b.empty()) {
   blk.erase(blk.begin() + j);
   return rebuild();
  }
  mx[j]= b.back(), add(j, -1);
 }
 int kth(int k) const {
  if(k >= total) return -1;
  int j= 0;
  for(int p= std::bit_floor(unsigned(fw.size())); p; p>>= 1)
   if(j + p < int(fw.size()) && fw[j + p] <= k) j+= p, k-= fw[j];
  return blk[j][k];
 }
 int count_le(int k) const {
  const int j= find(k + 1);
  if(j == int(blk.size())) return total;
  return prefix(j) + (std::upper_bound(blk[j].begin(), blk[j].end(), k) - blk[j].begin());
 }
 int prev(int k) const {
  const int j= find(k + 1);
  if(j == int(blk.size())) return total ? mx.back() : -1;
  const auto it= std::upper_bound(blk[j].begin(), blk[j].end(), k);
  if(it != blk[j].begin()) return *(it - 1);
  return j ? mx[j - 1] : -1;
 }
 int next(int k) const {
  const int j= find(k);
  if(j == int(blk.size())) return -1;
  return *std::lower_bound(blk[j].begin(), blk[j].end(), k);
 }
};
}
struct Solver {
 os_blocks::Set s;
 explicit Solver(const vector<int>& a): s(a) {}
 void insert(int x) { s.insert(x); }
 void erase(int x) { s.erase(x); }
 int kth(int k) const { return s.kth(k); }
 int count_le(int x) const { return s.count_le(x); }
 int prev(int x) const { return s.prev(x); }
 int next(int x) const { return s.next(x); }
};
