#pragma once
// 静的 Rectangle Add Rectangle Sum。
//
// 加算 (l, d, r, u, w) の、左下 (X, Y) までの和への寄与は
//   w * (max(0, X - l) - max(0, X - r)) * (max(0, Y - d) - max(0, Y - u))
// で、展開すると (a, b) = (l, d), (l, u), (r, d), (r, u) の 4 点に符号 +, -, -, +
// で置いた s * (X - a) * (Y - b) の和になる。x = a で点を有効にしながら x の昇順に
// クエリを処理すると、S(X, Y) は b <= Y の点についての
//   s*X*Y - s*b*X - s*a*Y + s*a*b
// の和になる。b を圧縮した BIT にこの 4 つの係数を持たせて足す。
// クエリ (l, d, r, u) の答えは S(r, u) - S(l, u) - S(r, d) + S(l, d)。
#include <algorithm>
#include <array>
#include <vector>

struct Solver {
  static constexpr int MOD = 998244353;

  std::vector<std::array<int, 5>> adds;
  std::vector<std::array<int, 4>> queries;
  std::vector<int> ans;

  Solver(const std::vector<std::array<int, 5>> &adds_,
         const std::vector<std::array<int, 4>> &queries_)
      : adds(adds_), queries(queries_) {}

  // b を添字にした BIT。係数 (X*Y, X, Y, 1) を別々の配列で持つ。
  struct BIT {
    int n = 0;
    std::vector<int> xy, x, y, c;

    void init(int n_) {
      n = n_;
      xy.assign(n + 1, 0);
      x.assign(n + 1, 0);
      y.assign(n + 1, 0);
      c.assign(n + 1, 0);
    }

    void add(int i, int vxy, int vx, int vy, int vc) {
      for (; i <= n; i += i & -i) {
        int &a = xy[i];
        a += vxy;
        if (a >= MOD) a -= MOD;
        int &b = x[i];
        b += vx;
        if (b >= MOD) b -= MOD;
        int &d = y[i];
        d += vy;
        if (d >= MOD) d -= MOD;
        int &e = c[i];
        e += vc;
        if (e >= MOD) e -= MOD;
      }
    }

    void sum(int i, int &sxy, int &sx, int &sy, int &sc) const {
      sxy = sx = sy = sc = 0;
      for (; i > 0; i -= i & -i) {
        sxy += xy[i];
        if (sxy >= MOD) sxy -= MOD;
        sx += x[i];
        if (sx >= MOD) sx -= MOD;
        sy += y[i];
        if (sy >= MOD) sy -= MOD;
        sc += c[i];
        if (sc >= MOD) sc -= MOD;
      }
    }
  };

  void run() {
    const int n = (int)adds.size();
    ans.assign(queries.size(), 0);

    // 加算の角の y を集めて圧縮する。
    std::vector<int> ys;
    ys.reserve(2 * n);
    for (const auto &e : adds) {
      ys.push_back(e[1]);
      ys.push_back(e[3]);
    }
    std::sort(ys.begin(), ys.end());
    ys.erase(std::unique(ys.begin(), ys.end()), ys.end());

    // x = a で有効になる角。(a, b) に s * (X - a) * (Y - b) を足す。
    struct Event {
      int a, bi;
      int vxy, vx, vy, vc;
    };
    std::vector<Event> events;
    events.reserve(4 * n);
    for (const auto &e : adds) {
      const int w = e[4];
      auto put = [&](int a, int b, int sign) {
        const int s = sign > 0 ? w : (MOD - w) % MOD;
        const long long am = a % MOD, bm = b % MOD;
        Event ev;
        ev.a = a;
        ev.bi = int(std::lower_bound(ys.begin(), ys.end(), b) - ys.begin()) + 1;
        ev.vxy = s;
        ev.vx = int((MOD - (long long)s * bm % MOD) % MOD);  // -s*b
        ev.vy = int((MOD - (long long)s * am % MOD) % MOD);  // -s*a
        ev.vc = int((long long)s * am % MOD * bm % MOD);     // s*a*b
        events.push_back(ev);
      };
      put(e[0], e[1], +1);
      put(e[0], e[3], -1);
      put(e[2], e[1], -1);
      put(e[2], e[3], +1);
    }
    std::sort(events.begin(), events.end(),
              [](const Event &p, const Event &q) { return p.a < q.a; });

    // 各クエリを 4 つの S に分ける。
    struct Point {
      int x, y;  // S(x, y)
      int sign, qi;
    };
    std::vector<Point> points;
    points.reserve(4 * queries.size());
    for (int i = 0; i < (int)queries.size(); ++i) {
      const auto &e = queries[i];
      points.push_back({e[2], e[3], +1, i});  // +S(r, u)
      points.push_back({e[0], e[3], -1, i});  // -S(l, u)
      points.push_back({e[2], e[1], -1, i});  // -S(r, d)
      points.push_back({e[0], e[1], +1, i});  // +S(l, d)
    }
    std::sort(points.begin(), points.end(),
              [](const Point &p, const Point &q) { return p.x < q.x; });

    BIT bit;
    bit.init((int)ys.size());
    int j = 0;
    for (const auto &pt : points) {
      while (j < (int)events.size() && events[j].a <= pt.x) {
        const Event &ev = events[j++];
        bit.add(ev.bi, ev.vxy, ev.vx, ev.vy, ev.vc);
      }
      const int k =
          int(std::upper_bound(ys.begin(), ys.end(), pt.y) - ys.begin());
      int sxy, sx, sy, sc;
      bit.sum(k, sxy, sx, sy, sc);
      const long long xm = pt.x % MOD, ym = pt.y % MOD;
      const long long val = ((long long)sxy * xm % MOD * ym +
                             (long long)sx * xm + (long long)sy * ym + sc) %
                            MOD;
      int &a = ans[pt.qi];
      if (pt.sign > 0) {
        a += (int)val;
        if (a >= MOD) a -= MOD;
      } else {
        a -= (int)val;
        if (a < 0) a += MOD;
      }
    }
  }

  const std::vector<int> &answers() const { return ans; }
};
