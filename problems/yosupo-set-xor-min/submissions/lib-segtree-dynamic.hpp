#pragma once
#include "common.hpp"
#include "mylib/data_structure/SegmentTree_Dynamic.hpp"

// 動的セグメント木。2^30 の添字空間に対して、触ったところだけノードを作る。
// 経路を 1 本作るごとに高さぶんのノードが要る。
struct Solver {
  SegmentTree_Dynamic<CountSum> seg;

  void insert(int x) { seg.set(x, 1); }

  void erase(int x) { seg.set(x, 0); }

  int xor_min(int x) {
    return seg.find_first(0, [](int s) { return s >= 1; }, x);
  }
};
