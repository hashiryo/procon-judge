#pragma once
// ライブラリを使う提出が共有するもの。格子の外に出ている点を中へ寄せる費用と、
// 各マスに何個来たかを数えるところは同じなので、ここに置く。
#include "pj.hpp"

struct Reduced {
  i64 base = 0;                // 格子の中へ寄せるまでにかかる費用
  vector<array<int, 2>> cell;  // 各列の 2 マスに来た個数 (空きを -1 で表す)
};

inline Reduced reduce(int n, const vector<array<i64, 2>> &pts) {
  Reduced r;
  r.cell.assign(n, {-1, -1});
  for (auto &p : pts) {
    i64 x = p[0], y = p[1];
    int col;
    if (x < 1) r.base += 1 - x, col = 0;
    else if (x > n) r.base += x - n, col = n - 1;
    else col = (int)x - 1;
    if (y < 1) ++r.cell[col][0], r.base += 1 - y;
    else if (y > 2) ++r.cell[col][1], r.base += y - 2;
    else ++r.cell[col][(int)y - 1];
  }
  return r;
}
