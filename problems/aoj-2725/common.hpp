#pragma once
// ライブラリを使う提出が共有するもの。時刻の昇順に並べ替えるところまでは
// どちらも同じなので、ここに置く。
#include <algorithm>
#include <numeric>
#include "pj.hpp"

// items を f (時刻) の昇順に見る順番を返す。
inline vector<int> order_by_time(const vector<array<i64, 3>> &items) {
  vector<int> ord(items.size());
  std::iota(ord.begin(), ord.end(), 0);
  std::sort(ord.begin(), ord.end(),
            [&](int i, int j) { return items[i][2] < items[j][2]; });
  return ord;
}
