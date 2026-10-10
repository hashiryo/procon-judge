#pragma once
// blocks の二分探索 (塊の最大の列と塊の中) を、std::lower_bound と std::upper_bound から、分岐の無い形に替えたもの。
// 範囲を半分ずつ縮めるときの選び方を条件付きの足し算にして、比べた結果を待たずに次の位置を読めるようにする。
// 1 回目と 2 回目では、blocks の x64-gcc が x64-clang の 2 倍ほどかかるケースがあった (乱択の値で x 以下の個数を多く
// 問うケースで 59.57 ms と 30.79 ms)。ほかは blocks と同じ。
#include <algorithm>
#include <bit>
#include <vector>
namespace os_blocks_bl {
// 昇順の a[0, n) のうち x 未満のものの個数 (lower_bound の位置)。
inline int lb(const int* a, int n, int x) {
 if(n == 0) return 0;
 const int* b= a;
 for(; n > 1; n-= n / 2) b+= (b[n / 2 - 1] < x) * (n / 2);
 return (b - a) + (*b < x);
}
// 昇順の a[0, n) のうち x 以下のものの個数 (upper_bound の位置)。
inline int ub(const int* a, int n, int x) {
 if(n == 0) return 0;
 const int* b= a;
 for(; n > 1; n-= n / 2) b+= (b[n / 2 - 1] <= x) * (n / 2);
 return (b - a) + (*b <= x);
}
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
 int find(int k) const { return lb(mx.data(), mx.size(), k); }
 void insert(int k) {
  if(blk.empty()) {
   blk.push_back({k}), total= 1;
   return rebuild();
  }
  int j= find(k);
  if(j == int(blk.size())) --j;
  std::vector<int>& b= blk[j];
  const auto it= b.begin() + lb(b.data(), b.size(), k);
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
  const auto it= b.begin() + lb(b.data(), b.size(), k);
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
  return prefix(j) + ub(blk[j].data(), blk[j].size(), k);
 }
 int prev(int k) const {
  const int j= find(k + 1);
  if(j == int(blk.size())) return total ? mx.back() : -1;
  if(const int p= ub(blk[j].data(), blk[j].size(), k)) return blk[j][p - 1];
  return j ? mx[j - 1] : -1;
 }
 int next(int k) const {
  const int j= find(k);
  if(j == int(blk.size())) return -1;
  return blk[j][lb(blk[j].data(), blk[j].size(), k)];
 }
};
}
struct Solver {
 os_blocks_bl::Set s;
 explicit Solver(const vector<int>& a): s(a) {}
 void insert(int x) { s.insert(x); }
 void erase(int x) { s.erase(x); }
 int kth(int k) const { return s.kth(k); }
 int count_le(int x) const { return s.count_le(x); }
 int prev(int x) const { return s.prev(x); }
 int next(int x) const { return s.next(x); }
};
