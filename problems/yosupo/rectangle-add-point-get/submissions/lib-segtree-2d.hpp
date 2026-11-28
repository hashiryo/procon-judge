#pragma once
// 今の Library の 2 次元セグメント木 (SegmentTree_2D) を、四隅の差分配列に使う。
// 点の加算と長方形の和ができるので、長方形の加算を四隅の点加算に、点の取得を
// 原点からの長方形の和に直す。圧縮と構築は run() の中で行う (reference.hpp と揃える)。
#include <optional>
#include "pj.hpp"
#include "mylib/data_structure/SegmentTree_2D.hpp"

struct AddMonoid {
  using T = long long;
  static T ti() { return 0; }
  static T op(T a, T b) { return a + b; }
};

struct Solver {
  Solver(const vector<array<i64, 5>> &rect, const vector<array<i64, 6>> &query)
      : rect_(rect), query_(query) {}

  void run();
  const vector<i64> &answers() const { return ans_; }

 private:
  using Tree = SegmentTree_2D<int, AddMonoid>;

  const vector<array<i64, 5>> &rect_;
  const vector<array<i64, 6>> &query_;
  std::optional<Tree> st_;
  vector<i64> ans_;

  void add_point(i64 x, i64 y, i64 w) { st_->mul((int)x, (int)y, w); }

  void push_corners(i64 l, i64 d, i64 r, i64 u, i64 w) {
    add_point(l, d, w);
    add_point(r, d, -w);
    add_point(l, u, -w);
    add_point(r, u, w);
  }
};

inline void Solver::run() {
  // 四隅 {x, y} を集める。値はすべて 0 で作り、クエリの処理で足していく。
  vector<pair<array<int, 2>, long long>> pts;
  pts.reserve(4 * (rect_.size() + query_.size()));
  auto collect = [&pts](i64 l, i64 d, i64 r, i64 u) {
    pts.push_back({{(int)l, (int)d}, 0});
    pts.push_back({{(int)r, (int)d}, 0});
    pts.push_back({{(int)l, (int)u}, 0});
    pts.push_back({{(int)r, (int)u}, 0});
  };
  for (const auto &e : rect_) collect(e[0], e[1], e[2], e[3]);
  for (const auto &e : query_)
    if (e[0] == 0) collect(e[1], e[2], e[3], e[4]);
  st_.emplace(pts);

  for (const auto &e : rect_) push_corners(e[0], e[1], e[2], e[3], e[4]);
  ans_.clear();
  ans_.reserve(query_.size());
  for (const auto &e : query_) {
    if (e[0] == 0) push_corners(e[1], e[2], e[3], e[4], e[5]);
    else ans_.push_back(st_->prod(-1, (int)e[1] + 1, -1, (int)e[2] + 1));
  }
}
