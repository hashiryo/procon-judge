#pragma once
// 全域最小カットの 4 問 (luogu-P5632、hdu-6081、poj-2914、hdu-3691) の提出が共有する Solver。頂点 n 個と無向辺
// {u, v, 重み} (頂点は 0 始まり) を受け、全域最小カットの値を返す。中身は提出が GMC_CORE に名前を入れて選ぶ
// (_shared/gmincut の gmc::sw_dense、gmc::sw_heap、gmc::noi<false>、gmc::noi<true> など)。
#include <array>
#include <vector>

struct Solver {
  int n;
  const vector<array<i64, 3>> &e;
  Solver(int n, const vector<array<i64, 3>> &e) : n(n), e(e) {}
  i64 run() { return GMC_CORE(n, e); }
};
