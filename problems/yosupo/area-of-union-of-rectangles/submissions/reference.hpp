#pragma once
// 長方形の和集合の面積。x で走査する走査線。
//
// y を座標圧縮して、区間 [ys[i], ys[i+1]) が何枚の長方形に覆われているかを木で持つ。
// 木の節点は「その節点の区間を丸ごと覆う長方形の枚数 cnt」と「覆われた長さ len」を持ち、
//   cnt > 0 なら len は区間の全体の長さ、そうでなければ子の len の和
// になる。x が同じ事象 (長方形の始まりと終わり) をまとめて処理してから、
// 覆われた長さ × 次の x までの幅 を足す。
#include <algorithm>
#include <utility>
#include <vector>

struct Solver {
  int n;
  std::vector<long long> l, d, r, u;
  long long area_ = 0;

  // --- 覆われた長さを持つ木 (葉が y の区間) ---
  std::vector<long long> ys;    // 座標圧縮した y。区間 i は [ys[i], ys[i+1])。
  std::vector<int> cnt;         // 丸ごと覆われた回数
  std::vector<long long> len;   // 覆われた長さ
  std::vector<long long> span;  // 節点の区間の全体の長さ
  int size = 1;                 // 葉の数 (2 の冪)

  struct Event {
    long long x;
    int lo, hi, delta;  // y の区間 [lo, hi) を delta 枚だけ動かす
  };
  std::vector<Event> ev;

  Solver(int n_, std::vector<long long> l_, std::vector<long long> d_,
         std::vector<long long> r_, std::vector<long long> u_)
      : n(n_), l(std::move(l_)), d(std::move(d_)), r(std::move(r_)), u(std::move(u_)) {}

  void pull(int i) {
    if (cnt[i] > 0) len[i] = span[i];
    else if (i >= size) len[i] = 0;
    else len[i] = len[i * 2] + len[i * 2 + 1];
  }

  // [ql, qr) の cnt を v だけ動かす。
  void add(int ql, int qr, int v) {
    int a = ql + size, b = qr + size;
    const int a0 = a, b0 = b - 1;
    for (; a < b; a >>= 1, b >>= 1) {
      if (a & 1) {
        cnt[a] += v;
        pull(a);
        ++a;
      }
      if (b & 1) {
        --b;
        cnt[b] += v;
        pull(b);
      }
    }
    // 覆った節点の先祖を下から直す。境目の葉の先祖をたどれば全部含まれる。
    for (int i = a0 >> 1; i; i >>= 1) pull(i);
    for (int i = b0 >> 1; i; i >>= 1) pull(i);
  }

  void run() {
    area_ = 0;
    if (n <= 0) return;
    ys.clear();
    ys.reserve(size_t(n) * 2);
    for (int i = 0; i < n; ++i) {
      ys.push_back(d[i]);
      ys.push_back(u[i]);
    }
    std::sort(ys.begin(), ys.end());
    ys.erase(std::unique(ys.begin(), ys.end()), ys.end());
    const int m = int(ys.size()) - 1;  // 区間の数

    size = 1;
    while (size < m) size <<= 1;
    cnt.assign(size_t(size) * 2, 0);
    len.assign(size_t(size) * 2, 0);
    span.assign(size_t(size) * 2, 0);
    for (int i = 0; i < m; ++i) span[size + i] = ys[i + 1] - ys[i];
    for (int i = size - 1; i >= 1; --i) span[i] = span[i * 2] + span[i * 2 + 1];

    ev.resize(size_t(n) * 2);
    for (int i = 0; i < n; ++i) {
      const int lo = int(std::lower_bound(ys.begin(), ys.end(), d[i]) - ys.begin());
      const int hi = int(std::lower_bound(ys.begin(), ys.end(), u[i]) - ys.begin());
      ev[size_t(i) * 2] = {l[i], lo, hi, +1};
      ev[size_t(i) * 2 + 1] = {r[i], lo, hi, -1};
    }
    std::sort(ev.begin(), ev.end(),
              [](const Event &p, const Event &q) { return p.x < q.x; });

    long long a = 0;
    size_t i = 0;
    while (i < ev.size()) {
      const long long x = ev[i].x;
      for (; i < ev.size() && ev[i].x == x; ++i) add(ev[i].lo, ev[i].hi, ev[i].delta);
      if (i < ev.size()) a += len[1] * (ev[i].x - x);
    }
    area_ = a;
  }

  long long area() const { return area_; }
};
