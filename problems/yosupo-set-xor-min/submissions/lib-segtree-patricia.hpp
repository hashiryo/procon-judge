#pragma once
#include "common.hpp"
#include "mylib/data_structure/SegmentTree_Patricia.hpp"

// パトリシア木。枝分かれの無い経路を 1 ノードに畳むので、要素が疎なときに
// ノード数が減る。この問題は値が散らばるので、動的セグメント木との差が出る。
struct Solver {
  SegmentTree_Patricia<CountSum> seg;

  void insert(int x) { seg.set(x, 1); }

  void erase(int x) { seg.set(x, 0); }

  int xor_min(int x) {
    return seg.find_first(0, [](int s) { return s >= 1; }, x);
  }
};
