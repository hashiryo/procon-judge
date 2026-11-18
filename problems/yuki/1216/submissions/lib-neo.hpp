#pragma once
#include <algorithm>
#include <limits>
#include <set>
#include "pj.hpp"
#include "neo/data_structure/PointAddRectangleSum.hpp"
#include "neo/graph/HeavyLightDecomposition.hpp"

// common.hpp の LanternSolver と同じ解き方を、NeoLibrary で書いたもの。木は HeavyLightDecomposition で列に潰し、2 次元の点への
// 1 点の加算と長方形の総和は PointAddRectangleSum (点の座標を先に渡しておく wavelet matrix と BIT)。common.hpp は今の Library の
// HLD を読むので、名前がぶつからないようここでは読まない。
struct Solver {
  // delta != 0 なら点 (x, y) への加算。delta == 0 なら [x0, x1) x [0, y] の総和の問い合わせ。
  struct Event {
    int delta;
    i64 x0, x1, y;
  };
  int n;
  vector<array<i64, 3>> edges;
  vector<array<i64, 4>> qs;
  vector<i64> ans;
  Solver(int n, const vector<array<i64, 3>> &edges, const vector<array<i64, 4>> &qs) : n(n), edges(edges), qs(qs) {}
  void run() {
    vector<array<int, 2>> es(n - 1);
    for (int i = 0; i < n - 1; ++i) es[i] = {(int)edges[i][0], (int)edges[i][1]};
    HeavyLightDecomposition tree(n, es, 0);
    // 根からの距離。辺の重みを子の側の頂点に置き、位置の順 (親が子より前) に足す。
    vector<i64> w(n, 0), dep(n, 0);
    for (int i = 0; i < n - 1; ++i) w[tree.parent(es[i][0]) == es[i][1] ? es[i][0] : es[i][1]] = edges[i][2];
    for (int i = 1; i < n; ++i) {
      const int v = tree.to_vertex(i);
      dep[v] = dep[tree.parent(v)] + w[v];
    }
    set<array<i64, 2>> points;
    vector<Event> events;
    vector<array<int, 2>> segs;
    for (auto &q : qs) {
      const int v = (int)q[1];
      const i64 t = q[2], l = q[3];
      if (q[0] == 0) {
        i64 x = tree.to_seq(v), y = t + dep[v];
        events.push_back({1, 0, x, y});
        points.insert({x, y});
        // 根へ向かって、灯籠が消えたあとに通る最初の街 u を探す。根から v への道の区間は根の側から順に来るので、逆から見る。
        segs.clear();
        tree.path(0, v, [&](int a, int b) { segs.push_back({a, b}); });
        int u = -1;
        for (int i = (int)segs.size(); i--;) {
          auto [a, b] = segs[i];
          if (dep[v] - dep[tree.to_vertex(a)] <= l) continue;
          for (++b; b - a > 1;) {
            const int m = (a + b) / 2;
            (dep[v] - dep[tree.to_vertex(m)] > l ? a : b) = m;
          }
          u = tree.to_vertex(a);
          break;
        }
        if (u != -1) {
          x = tree.to_seq(u);
          events.push_back({-1, 0, x, y});
          points.insert({x, y});
        }
      } else {
        auto [lo, hi] = tree.subtree(v);
        events.push_back({0, lo, hi, t + dep[v]});
      }
    }
    vector<array<i64, 2>> pts(points.begin(), points.end());
    vector<i64> xs(pts.size()), ys(pts.size()), ws(pts.size(), 0);
    for (size_t i = 0; i < pts.size(); ++i) xs[i] = pts[i][0], ys[i] = pts[i][1];
    PointAddRectangleSum<i64, i64> rs(xs, ys, ws);
    ans.clear();
    for (auto &e : events) {
      if (e.delta == 0) ans.push_back(rs.sum(e.x0, e.x1, std::numeric_limits<i64>::min(), e.y + 1));
      else rs.add(int(std::lower_bound(pts.begin(), pts.end(), array<i64, 2>{e.x1, e.y}) - pts.begin()), e.delta);
    }
  }
  const vector<i64> &answer() const { return ans; }
};
