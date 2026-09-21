#pragma once
// ライブラリを使う提出が共有するもの。S + T の降順に生徒と質問を並べて、質問の
// 時点で条件を満たす生徒だけが 2 次元の点集合に入っている形にするところは、
// 2 次元の構造を使う実装で同じなので、ここに置く。
#include <algorithm>
#include <set>
#include "pj.hpp"

struct RangeCount {
  using T = int;
  static T ti() { return 0; }
  static T op(T l, T r) { return l + r; }
};

// 座標の上限。S, T <= 1e9 なので S + T も int に収まる。
constexpr int INF = 0x7fffffff;

// {-(S + T) または -Z, 質問の番号 (生徒は -1), x, y}。昇順に並べると Z の大きい方
// から処理され、同じ値なら生徒が先に入る (S + T >= Z は等号を含む)。
struct Offline {
  set<array<int, 2>> points;
  vector<array<int, 4>> events;
};

inline Offline prepare(const vector<array<int, 2>> &st, const vector<array<int, 3>> &qs) {
  Offline o;
  for (auto &e : st) {
    o.points.insert({e[0], e[1]});
    o.events.push_back({-(e[0] + e[1]), -1, e[0], e[1]});
  }
  for (int i = 0; i < (int)qs.size(); ++i) o.events.push_back({-qs[i][2], i, qs[i][0], qs[i][1]});
  std::sort(o.events.begin(), o.events.end());
  return o;
}

// Engine は次を実装する。
//   Engine(const set<array<int, 2>> &points);   // 点の集合。値は 0
//   void add(int x, int y);                      // 点 (x, y) を 1 増やす
//   int count(int x, int y);                     // x 以上かつ y 以上の点の値の和
template <class Engine> struct OfflineSolver {
  vector<array<int, 2>> st;
  vector<array<int, 3>> qs;
  vector<i64> ans;

  OfflineSolver(const vector<array<int, 2>> &st, const vector<array<int, 3>> &qs) : st(st), qs(qs) {}

  void run() {
    Offline o = prepare(st, qs);
    Engine eng(o.points);
    ans.assign(qs.size(), 0);
    for (auto [z, i, x, y] : o.events) {
      if (i < 0) eng.add(x, y);
      else ans[i] = eng.count(x, y);
    }
  }

  const vector<i64> &answer() const { return ans; }
};
