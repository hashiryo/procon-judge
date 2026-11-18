#pragma once
// _shared/kuhn/inc_matching.hpp の IncMatching を使う提出が共有するもの。辺 (x, y) があるかは呼ぶ側の集合で持ち、
// 切り替えのたびに add_edge か erase_edge を呼んでから augment() を 1 回呼ぶ。辺を 1 本足しても抜いても最大マッチングの
// 大きさは高々 1 しか変わらないので、増加路を 1 本流せば最大に戻る。初めのマッチングは構築の中で augment() を繰り返して
// 作る。Match は提出が決めてから include する。
#include <unordered_set>

struct Solver {
  int n;
  Match m;
  unordered_set<long long> on;
  Solver(int n, const vector<array<int, 2>> &edges) : n(n), m(n, n, edges) {
    on.reserve(edges.size() * 2 + 16);
    for (auto &e : edges) on.insert((long long)e[0] * n + e[1]);
    while (m.augment()) {}
  }
  bool toggle(int x, int y) {
    const long long k = (long long)x * n + y;
    if (on.erase(k)) m.erase_edge(x, y);
    else on.insert(k), m.add_edge(x, y);
    m.augment();
    return m.size() == n;
  }
};
