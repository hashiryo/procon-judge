#pragma once
// ライブラリを使う提出が共有するもの。
#include <set>
#include "pj.hpp"

// 焼き中と焼き上がりを 1 つの値にまとめて持つ。疎な構造はこれを 1 点の値に
// 載せる。密な構造 (BIT) は 2 本に分けて持つ方が軽いので使わない。
struct PairSum {
  using T = pair<int, int>;
  static T ti() { return {0, 0}; }
  static T op(const T &l, const T &r) {
    return {l.first + r.first, l.second + r.second};
  }
};

// 触られる座標の一覧。疎な構造は先に固定しないと載らない。
inline std::set<array<int, 2>> touched(const vector<array<int, 5>> &events) {
  std::set<array<int, 2>> s;
  for (auto &e : events)
    if (e[0] != 2) s.insert({e[1], e[2]});
  return s;
}
