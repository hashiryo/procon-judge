#pragma once
// 2 次元の差分配列を 4 係数の 2 次元 BIT に載せる。オンラインでも使える形。
//
// 長方形 [l, r) x [d, u) への w の加算を、四隅 (l, d) += w、(r, d) -= w、(l, u) -= w、
// (r, u) += w の点加算に直す。点 (x, y) の値は x 以下 y 以下の四隅の和で、
// 長方形 [L, R) x [D, U) の総和は P(R, U) - P(L, U) - P(R, D) + P(L, D) になる。
// ここで P(X, Y) は x < X かつ y < Y の四隅 (a, b, D) についての
//   D (X - a) (Y - b) = X Y D - X D b - Y D a + D a b
// の和なので、D、D b、D a、D a b の 4 つを 2 次元 BIT に持てばよい。
// 4 つは 1 つのセルに並べて置き、添字の計算を共有する。
#include <algorithm>
#include <array>
#include <vector>
#include "pj.hpp"

namespace bit2d_4coef_internal {
struct Lookup {
  std::vector<int> v;
  unsigned lo = 0, sh = 0;
  std::vector<int> t;
  void build() {
    t.clear();
    if (v.empty()) return;
    lo = (unsigned)v[0];
    const unsigned d = (unsigned)(v.back() - v[0]);
    sh = 0;
    while ((d >> sh) >= (1u << 16)) ++sh;
    const int K = (int)(d >> sh) + 1, m = (int)v.size();
    t.resize(K + 1);
    for (int i = 0, k = 0; k <= K; ++k) {
      while (i < m && (int)(((unsigned)(v[i] - (int)lo)) >> sh) < k) ++i;
      t[k] = i;
    }
  }
  int lower(int x) const {
    const int m = (int)v.size();
    if (m == 0 || x <= v[0]) return 0;
    if (v.back() < x) return m;
    int a = t[(unsigned)(x - (int)lo) >> sh];
    while (v[a] < x) ++a;
    return a;
  }
};
}  // namespace bit2d_4coef_internal

struct Solver {
  using u32 = unsigned;
  using u64 = unsigned long long;
  static constexpr u32 MOD = 998244353;

  struct Quad {
    u32 d, db, da, dab;
  };

  Solver(const vector<array<int, 5>> &adds, const vector<array<int, 4>> &queries)
      : adds_(adds), queries_(queries) {}

  void run();
  const vector<int> &answers() const { return ans_; }

 private:
  const vector<array<int, 5>> &adds_;      // {l, d, r, u, w}
  const vector<array<int, 4>> &queries_;   // {l, d, r, u}

  bit2d_4coef_internal::Lookup xs_;
  vector<int> yv_, yoff_, ylen_;
  vector<Quad> bit_;
  vector<int> boff_;
  int nx_ = 0;
  vector<int> ans_;

  static u32 mul_mod(u64 a, u64 b) { return (u32)(a * b % MOD); }

  void add_corner(int a, int b, u32 d) {
    const u32 da = mul_mod((u64)(a % (int)MOD), d);
    const u32 db = mul_mod((u64)(b % (int)MOD), d);
    const u32 dab = mul_mod((u64)(a % (int)MOD), db);
    for (int i = xs_.lower(a) + 1; i <= nx_; i += i & -i) {
      const int n = ylen_[i], base = yoff_[i];
      int j = (int)(std::lower_bound(yv_.begin() + base, yv_.begin() + base + n, b) - yv_.begin()) - base + 1;
      Quad *q = bit_.data() + boff_[i];
      for (; j <= n; j += j & -j) {
        u32 &x0 = q[j].d, &x1 = q[j].db, &x2 = q[j].da, &x3 = q[j].dab;
        x0 += d;
        if (x0 >= MOD) x0 -= MOD;
        x1 += db;
        if (x1 >= MOD) x1 -= MOD;
        x2 += da;
        if (x2 >= MOD) x2 -= MOD;
        x3 += dab;
        if (x3 >= MOD) x3 -= MOD;
      }
    }
  }

  // x < X かつ y < Y の四隅についての (D, D b, D a, D a b) の和。
  void prefix(int X, int Y, u64 &s1, u64 &s2, u64 &s3, u64 &s4) const {
    s1 = s2 = s3 = s4 = 0;
    for (int i = xs_.lower(X); i > 0; i -= i & -i) {
      const int n = ylen_[i], base = yoff_[i];
      int j = (int)(std::lower_bound(yv_.begin() + base, yv_.begin() + base + n, Y) - yv_.begin()) - base;
      const Quad *q = bit_.data() + boff_[i];
      for (; j > 0; j -= j & -j) s1 += q[j].d, s2 += q[j].db, s3 += q[j].da, s4 += q[j].dab;
    }
  }

  u32 prefix_value(int X, int Y) const {
    u64 s1, s2, s3, s4;
    prefix(X, Y, s1, s2, s3, s4);
    const u64 xy = (u64)(X % (int)MOD) * (u64)(Y % (int)MOD) % MOD;
    i64 r = (i64)(xy * (s1 % MOD) % MOD);
    r -= (i64)((u64)(X % (int)MOD) * (s2 % MOD) % MOD);
    r -= (i64)((u64)(Y % (int)MOD) * (s3 % MOD) % MOD);
    r += (i64)(s4 % MOD);
    r %= (i64)MOD;
    if (r < 0) r += MOD;
    return (u32)r;
  }
};

inline void Solver::run() {
  // 四隅 {a, b, D} を集める。
  vector<array<int, 3>> cs;
  cs.reserve(4 * adds_.size());
  auto collect = [&cs](int l, int d, int r, int u, u32 w) {
    cs.push_back({l, d, (int)w});
    cs.push_back({r, d, (int)(w ? MOD - w : 0)});  // -w
    cs.push_back({l, u, (int)(w ? MOD - w : 0)});
    cs.push_back({r, u, (int)w});
  };
  for (const auto &e : adds_) collect(e[0], e[1], e[2], e[3], (u32)(e[4] % (int)MOD));

  // x を圧縮する。
  xs_.v.clear();
  xs_.v.reserve(cs.size());
  for (const auto &c : cs) xs_.v.push_back(c[0]);
  std::sort(xs_.v.begin(), xs_.v.end());
  xs_.v.erase(std::unique(xs_.v.begin(), xs_.v.end()), xs_.v.end());
  xs_.build();
  nx_ = (int)xs_.v.size();

  // 四隅の y を、その x が通る x の節すべてに積む。
  vector<int> xi(cs.size());
  vector<int> cnt(nx_ + 2, 0);
  for (size_t j = 0; j < cs.size(); ++j) {
    xi[j] = xs_.lower(cs[j][0]) + 1;
    for (int i = xi[j]; i <= nx_; i += i & -i) ++cnt[i];
  }
  yoff_.assign(nx_ + 2, 0);
  for (int i = 1; i <= nx_; ++i) yoff_[i + 1] = yoff_[i] + cnt[i];
  yv_.resize(yoff_[nx_ + 1]);
  {
    vector<int> fill(yoff_.begin(), yoff_.end());
    for (size_t j = 0; j < cs.size(); ++j)
      for (int i = xi[j]; i <= nx_; i += i & -i) yv_[fill[i]++] = cs[j][1];
  }
  ylen_.assign(nx_ + 2, 0);
  {
    vector<int> start(nx_ + 2, 0);
    int w = 0;
    for (int i = 1; i <= nx_; ++i) {
      const int b = yoff_[i], e = yoff_[i + 1];
      std::sort(yv_.begin() + b, yv_.begin() + e);
      const int u = (int)(std::unique(yv_.begin() + b, yv_.begin() + e) - (yv_.begin() + b));
      start[i] = w;
      for (int j = 0; j < u; ++j) yv_[w + j] = yv_[b + j];
      ylen_[i] = u;
      w += u;
    }
    yv_.resize(w);
    yoff_ = std::move(start);
  }
  boff_.assign(nx_ + 2, 0);
  {
    int w = 0;
    for (int i = 1; i <= nx_; ++i) boff_[i] = w, w += ylen_[i] + 1;
    bit_.assign(w, Quad{0, 0, 0, 0});
  }

  for (const auto &c : cs) add_corner(c[0], c[1], (u32)c[2]);

  ans_.clear();
  ans_.reserve(queries_.size());
  for (const auto &q : queries_) {
    const int L = q[0], D = q[1], R = q[2], U = q[3];
    u64 a = prefix_value(R, U), b = prefix_value(L, U), c = prefix_value(R, D), d = prefix_value(L, D);
    i64 r = (i64)a - (i64)b - (i64)c + (i64)d;
    r %= (i64)MOD;
    if (r < 0) r += MOD;
    ans_.push_back((int)r);
  }
}
