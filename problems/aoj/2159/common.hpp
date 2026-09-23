#pragma once
// ライブラリを使う提出が共有するもの。解法はここに 1 つ置いて、提出は座標の
// 型を選ぶだけにする。誤差で答えが変わりうる問題なので、そこが見どころになる。
#include <set>
#include "pj.hpp"
#include "mylib/geometry/Line.hpp"

template <class R> struct GeoSolver {
  using P = geo::Point<R>;
  using L = geo::Line<R>;

  vector<array<i64, 2>> src;
  bool ans = false;

  explicit GeoSolver(const vector<array<i64, 2>> &src) : src(src) {}

  void run() {
    const int n = (int)src.size();
    vector<P> ps(n);
    std::set<P> s;
    for (int i = 0; i < n; ++i) ps[i] = P{R(src[i][0]), R(src[i][1])}, s.insert(ps[i]);

    // 全部が 1 直線に乗っていると、対称軸が無限にあるので別扱い。
    bool online = true;
    L m = geo::line_through(ps[0], ps[1]);
    for (int i = 2; i < n; ++i) online &= m.where(ps[i]) == 0;
    if (online) {
      ans = false;
      return;
    }

    // 軸があるなら、先頭 3 点のいずれかは軸の上か、他の点と対になる。
    vector<L> ls;
    for (int i = 3; i--;)
      for (int j = i + 1; j < n; ++j) ls.emplace_back(geo::bisector(ps[i], ps[j]));

    auto check = [&](const L &l) {
      int on = 0;
      auto ref = geo::reflect(l);
      for (const P &p : ps) {
        on += l.where(p) == 0;
        if (!s.count(ref(p))) return false;
      }
      return on <= 2;
    };

    ans = false;
    for (auto &l : ls) ans |= check(l);
  }

  bool answer() const { return ans; }
};
