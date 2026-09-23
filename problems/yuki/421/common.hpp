#pragma once
// ライブラリを使う提出が共有するもの。マス目を数えて隣り合うビターとホワイトの
// 組を辺にするところと、マッチングの数から幸福度を出す式はどの実装でも同じなので、
// ここに置く。比べたいのは 2 部マッチングを取る手段。
#include <algorithm>
#include "pj.hpp"

struct Chocolate {
  int cells = 0;                // 全マス数 (頂点番号の範囲)
  int bsize = 0, wsize = 0;     // 少ない方と多い方の枚数
  vector<array<int, 2>> edges;  // {ビターのマス, 隣のホワイトのマス}
};

inline Chocolate parse(const vector<string> &s) {
  const int n = (int)s.size(), m = (int)s[0].size();
  Chocolate c;
  c.cells = n * m;
  for (int i = 0; i < n; ++i)
    for (int j = 0; j < m; ++j) c.bsize += s[i][j] == 'b', c.wsize += s[i][j] == 'w';
  for (int i = 0; i < n; ++i)
    for (int j = 0; j < m; ++j)
      if ((i + j) & 1 && s[i][j] == 'b') {
        if (i > 0 && s[i - 1][j] == 'w') c.edges.push_back({i * m + j, (i - 1) * m + j});
        if (i + 1 < n && s[i + 1][j] == 'w') c.edges.push_back({i * m + j, (i + 1) * m + j});
        if (j > 0 && s[i][j - 1] == 'w') c.edges.push_back({i * m + j, i * m + j - 1});
        if (j + 1 < m && s[i][j + 1] == 'w') c.edges.push_back({i * m + j, i * m + j + 1});
      }
  if (c.bsize > c.wsize) std::swap(c.bsize, c.wsize);
  return c;
}

// 組で取れた x 対は 100、残りは色違いの同時食いで 10、余りは 1 ずつ。
inline i64 happiness(const Chocolate &c, int x) {
  return 100LL * x + 10LL * (c.bsize - x) + (c.wsize - c.bsize);
}
