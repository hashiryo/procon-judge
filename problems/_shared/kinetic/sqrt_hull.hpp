#pragma once
#include <algorithm>
#include <cstdint>
#include <vector>

// 区間の 1 次関数の加算 (y_i += a x_i + b、a の符号は任意) と区間の最大 (IsMax) か最小を、平方分割と塊ごとの凸包で行う。
// 各塊は、直線 (x_i, y_i) の上側 (IsMax) の凸包を傾きの順に持ち、塊に積んだ時刻 ba と定数 bb を持つ。塊の全体への作用は
// ba, bb をずらすだけでよく、端の塊は要素を直して凸包を組み直す。x は要素ごとに任意で、単調でなくてよい。
// kinetic segment tree の参照実装 (計算量の保証があり、時刻を前後に行き来させても壊れない) として使う。
template <bool IsMax> struct SqrtHull {
  using i64 = long long;
  using i128 = __int128_t;
  static constexpr i64 INF = i64(4) << 60;
  struct Line {
    i64 x, y;
  };
  int n = 0, B = 1, nb = 0;
  std::vector<i64> X, Y, ba, bb;
  std::vector<std::vector<int>> ord;    // 塊の添字を X の昇順に並べたもの
  std::vector<std::vector<Line>> hull;  // 上側の凸包 (x は狭義単調増加)
  std::vector<unsigned char> dirty;     // X が変わって ord を並べ直す必要がある

  void init(const std::vector<i64> &xs, const std::vector<i64> &ys) {
    n = int(xs.size());
    B = 1;
    while (B * B < n * 3) B <<= 1;  // 1 回の作用を O(B + n/B log B) にする
    if (B < 16) B = 16;
    nb = (n + B - 1) / B;
    X = xs, Y = ys;
    ba.assign(nb, 0), bb.assign(nb, 0);
    ord.assign(nb, {}), hull.assign(nb, {});
    dirty.assign(nb, 1);
    for (int b = 0; b < nb; ++b)
      for (int i = b * B; i < std::min(n, (b + 1) * B); ++i) ord[b].push_back(i);
    for (int b = 0; b < nb; ++b) rebuild(b);
  }

  void set(int i, i64 x, i64 y) {
    int b = i / B;
    // 今の時刻 ba での値を (x, y) にする。塊の作用は積んだまま、基礎の値だけ直す。
    X[i] = x, Y[i] = i64((i128)y - (i128)ba[b] * x - bb[b]);
    dirty[b] = 1;
    rebuild(b);
  }

  void add(int l, int r, i64 a, i64 b) {
    if (l >= r) return;
    if (l == 0 && r == n) {
      for (int t = 0; t < nb; ++t) ba[t] += a, bb[t] += b;
      return;
    }
    int bl = l / B, br = (r - 1) / B;
    if (bl == br) {
      for (int i = l; i < r; ++i) Y[i] += a * X[i] + b;
      rebuild(bl);
      return;
    }
    for (int i = l; i < (bl + 1) * B; ++i) Y[i] += a * X[i] + b;
    rebuild(bl);
    for (int t = bl + 1; t < br; ++t) ba[t] += a, bb[t] += b;
    for (int i = br * B; i < r; ++i) Y[i] += a * X[i] + b;
    rebuild(br);
  }

  i64 get(int l, int r) const {
    i64 res = IsMax ? -INF : INF;
    auto put = [&](i64 v) {
      if constexpr (IsMax) res = std::max(res, v);
      else res = std::min(res, v);
    };
    if (l == 0 && r == n) {
      for (int t = 0; t < nb; ++t) put(block_value(t));
      return res;
    }
    int bl = l / B, br = (r - 1) / B;
    if (bl == br) {
      for (int i = l; i < r; ++i) put(eval(i));
      return res;
    }
    for (int i = l; i < (bl + 1) * B; ++i) put(eval(i));
    for (int t = bl + 1; t < br; ++t) put(block_value(t));
    for (int i = br * B; i < r; ++i) put(eval(i));
    return res;
  }

  i64 all() const { return n == 0 ? (IsMax ? -INF : INF) : get(0, n); }

 private:
  i64 eval(int i) const {
    int b = i / B;
    return i64((i128)Y[i] + (i128)ba[b] * X[i] + bb[b]);
  }
  i64 block_value(int b) const {
    const auto &h = hull[b];
    if (h.empty()) return IsMax ? -INF : INF;
    i128 t = ba[b];
    int lo = 0, hi = int(h.size()) - 1;
    while (lo < hi) {
      int m = (lo + hi) >> 1;
      i128 v1 = (i128)h[m].x * t + h[m].y, v2 = (i128)h[m + 1].x * t + h[m + 1].y;
      if (v1 <= v2) lo = m + 1;
      else hi = m;
    }
    i128 v = (i128)h[lo].x * t + h[lo].y;
    if constexpr (IsMax) return i64(v) + bb[b];
    else return i64(-v) + bb[b];
  }
  static i128 cross(const Line &A, const Line &B, const Line &C) {
    return (i128)(B.x - A.x) * (C.y - A.y) - (i128)(B.y - A.y) * (C.x - A.x);
  }
  void rebuild(int b) {
    auto &o = ord[b];
    if (dirty[b]) {
      std::sort(o.begin(), o.end(), [&](int i, int j) { return X[i] != X[j] ? X[i] < X[j] : i < j; });
      dirty[b] = 0;
    }
    auto &h = hull[b];
    h.clear();
    int last = -1;
    auto run = [&](int i) {
      // 同じ傾きは 1 本だけ残す (最大なら y の大きいほう、最小なら小さいほう)。切片が良くなる線で
      // 置き換えるときは、置き換えた線でもう一度凸包を切り直す。
      if (last >= 0 && X[last] == X[i]) {
        bool win;
        if constexpr (IsMax) win = Y[i] > Y[last];
        else win = Y[i] < Y[last];
        if (!win) return;
        h.pop_back();
      }
      Line cur{IsMax ? X[i] : -X[i], IsMax ? Y[i] : -Y[i]};
      while (h.size() >= 2 && cross(h[h.size() - 2], h[h.size() - 1], cur) >= 0) h.pop_back();
      h.push_back(cur);
      last = i;
    };
    // 最小は y を反転して最大にするので、傾きは -X の昇順、つまり X の降順に並べる。
    if constexpr (IsMax) {
      for (int i : o) run(i);
    } else {
      for (int t = int(o.size()) - 1; t >= 0; --t) run(o[t]);
    }
  }
};
