#pragma once
// 長方形の加算と点の取得を、neo の RectangleAddPointGet (neo/base/ds/2d/RectangleAddPointGet.hpp、
// 四隅の差分配列を 2 次元 BIT に載せる形) で解く。手書きの参照 (reference.hpp) と書き比べる。
#include <array>
#include <vector>
#include "pj.hpp"
#include "neo/base/ds/2d/RectangleAddPointGet.hpp"

struct Solver {
  Solver(const vector<array<i64, 5>> &rect, const vector<array<i64, 6>> &query)
      : rect_(rect), query_(query) {}

  void run() {
    // 加算する長方形を全部集めてから作る。
    vector<array<int, 4>> all;
    all.reserve(rect_.size() + query_.size());
    for (const auto &e : rect_) all.push_back({(int)e[0], (int)e[1], (int)e[2], (int)e[3]});
    for (const auto &e : query_)
      if (e[0] == 0) all.push_back({(int)e[1], (int)e[2], (int)e[3], (int)e[4]});
    RectangleAddPointGet<int, i64> bit(all);
    for (const auto &e : rect_) bit.add((int)e[0], (int)e[1], (int)e[2], (int)e[3], e[4]);
    ans_.clear();
    ans_.reserve(query_.size());
    for (const auto &e : query_) {
      if (e[0] == 0)
        bit.add((int)e[1], (int)e[2], (int)e[3], (int)e[4], e[5]);
      else
        ans_.push_back(bit.get((int)e[1], (int)e[2]));
    }
  }

  const vector<i64> &answers() const { return ans_; }

 private:
  const vector<array<i64, 5>> &rect_;
  const vector<array<i64, 6>> &query_;
  vector<i64> ans_;
};
