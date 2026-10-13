#pragma once
#include "../common.hpp"
// =============================================================================
// stern_brocot_tree を xiaoziyao の SBT<T> class style (rational_approximation の
// 最速提出 https://judge.yosupo.jp/submission/290530 由来) で実装したバリアント。
//
// cuixiang19 流 (= 我々の fastest.hpp) の inline mediant ループに対し、
// SBT<T> class methods (get_path, from_path, lca, la, range) を使うアプローチ。
//
// 想定: vector<T> 経由になる get_path/from_path で多少の allocation オーバーヘッド、
//       但し API が抽象化されているのでコードが綺麗。
// =============================================================================
template<class T>
struct SBT {
 // 連分数展開風のパス分解。signed value: 正 = R 向き連長、負 = L 向き連長
 static std::vector<T> get_path(T a, T b) {
  std::vector<T> p;
  while (a > 1 || b > 1) {
   if (a > b) {
    T k = (a - 1) / b;
    a -= k * b;
    p.push_back(k);
   } else {
    T k = (b - 1) / a;
    b -= k * a;
    p.push_back(-k);
   }
  }
  return p;
 }
 // 逆: signed パス → fraction (a, b) を再構成
 static std::pair<T, T> from_path(std::vector<T> p, T a = 1, T b = 1) {
  std::reverse(p.begin(), p.end());
  for (T k : p) {
   if (k > 0) a += k * b;
   else b -= k * a;  // k < 0 なので減算
  }
  return {a, b};
 }
 static std::pair<T, T> lca(T a, T b, T c, T d) {
  auto p1 = get_path(a, b), p2 = get_path(c, d);
  int m = std::min(p1.size(), p2.size());
  std::vector<T> p;
  for (int i = 0; i < m; ++i) {
   T k = p1[i], l = p2[i];
   if ((k > 0) ^ (l > 0)) break;
   p.push_back(k > 0 ? std::min(k, l) : std::max(k, l));
   if (k != l) break;
  }
  return from_path(p);
 }
 static T depth(T a, T b) {
  T d = 0;
  for (T k : get_path(a, b)) d += std::abs(k);
  return d;
 }
 // depth-k ancestor: a/b へのパスを root から k 段降りた fraction
 static std::pair<T, T> la(T a, T b, T k) {
  if (k < 0 || k > depth(a, b)) return {-1, -1};
  auto p = get_path(a, b);
  std::vector<T> q;
  for (T d : p) {
   T ad = std::abs(d);
   if (ad <= k) { q.push_back(d); k -= ad; if (k == 0) break; }
   else { q.push_back(d > 0 ? k : -k); k = 0; break; }
  }
  return from_path(q);
 }
 // SB interval: a/b の左右隣 fraction (lx, ly, rx, ry)
 static std::tuple<T, T, T, T> range(T p, T q) {
  auto path = get_path(p, q);
  auto [a, b] = from_path(path, 0, 1);  // 左隣
  auto [c, d] = from_path(path, 1, 0);  // 右隣
  return {a, b, c, d};
 }
};

inline u32* run(const Input& in, u32* out) {
 for (const auto& qu : in.qs) {
  if (qu.type == ENCODE) {
   auto path = SBT<i64>::get_path(qu.x[0], qu.x[1]);
   u32* head = out++;
   if (!path.empty() && path[0] < 0) *out++ = 0;  // 最初の連が L なら、長さ 0 の R の連を置く
   for (i64 k : path) *out++ = (u32)std::abs(k);
   *head = (u32)(out - head - 1);
  } else if (qu.type == DECODE) {
   std::vector<i64> path;
   for (u32 i = 0; i < qu.len; ++i) {
    const i64 x = in.path[qu.off + i];
    if (x) path.push_back(i & 1 ? -x : x);
   }
   auto [a, b] = SBT<i64>::from_path(path);
   *out++ = (u32)a, *out++ = (u32)b;
  } else if (qu.type == LCA) {
   auto [p, q] = SBT<i64>::lca(qu.x[0], qu.x[1], qu.x[2], qu.x[3]);
   *out++ = (u32)p, *out++ = (u32)q;
  } else if (qu.type == ANCESTOR) {
   auto [p, q] = SBT<i64>::la(qu.x[1], qu.x[2], qu.x[0]);
   if (p < 0) *out++ = 0, *out++ = 0;
   else *out++ = (u32)p, *out++ = (u32)q;
  } else {  // RANGE
   auto [lx, ly, rx, ry] = SBT<i64>::range(qu.x[0], qu.x[1]);
   *out++ = (u32)lx, *out++ = (u32)ly, *out++ = (u32)rx, *out++ = (u32)ry;
  }
 }
 return out;
}
