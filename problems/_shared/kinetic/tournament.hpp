#pragma once
#include <algorithm>
#include <cstdint>
#include <queue>
#include <vector>

// 直線の集合 (a_i t + b_i) を持ち、時刻 t を前にだけ進めながら最大を問う kinetic tournament tree。
// 2 分木の節は「部分木で勝っている直線」と「その直線が入れ替わる次の整数時刻」を持ち、入れ替わりの時刻を優先度付きキューに
// 積む。問いの時刻までキューを進め、時刻が来た節だけを子から取り直して上へ伝える。古い予約は節ごとの世代で捨てる。
// 直線の集合が固定なら、時刻を最後まで進める間に入れ替わる回数は全体で O(N log N) (各部分木の上側の凸包の大きさの和)。
// 点の更新は道の O(log N) 節の取り直しで、更新の間の回数は同じ評価になる。問いは O(1) で、進める時刻の分だけイベントを
// 処理する。maspy の焼き鈍しの形 (beats) と違い、時刻の幅を持たずにイベントで進める。
struct KineticTournament {
  using i64 = long long;
  using i128 = __int128_t;
  static constexpr i64 INF = i64(4) << 60;
  int n = 0, sz = 1;
  std::vector<i64> A, B;
  std::vector<int> win;
  std::vector<i64> evt;
  std::vector<unsigned long long> gen;
  struct Ev {
    i64 t;
    int k;
    unsigned long long g;
  };
  struct Cmp {
    bool operator()(const Ev &x, const Ev &y) const { return x.t > y.t; }
  };
  std::priority_queue<Ev, std::vector<Ev>, Cmp> pq;
  i64 now = 0;

  void init(const std::vector<i64> &as, const std::vector<i64> &bs, i64 t0 = 0) {
    n = int(as.size());
    A = as, B = bs;
    sz = 1;
    while (sz < std::max(n, 1)) sz <<= 1;
    win.assign(2 * sz, -1), evt.assign(2 * sz, INF), gen.assign(2 * sz, 0);
    for (int i = 0; i < n; ++i) win[sz + i] = i;
    now = t0;
    for (int k = sz - 1; k >= 1; --k) pull(k, now), publish(k);
  }

  void set(int i, i64 a, i64 b) {
    A[i] = a, B[i] = b;
    for (int v = (sz + i) >> 1; v >= 1; v >>= 1) pull(v, now), publish(v);
  }

  i64 query(i64 t) {
    advance(t);
    int w = win[1];
    return w < 0 ? -INF : i64(eval(w, t));
  }

  // 時刻 t まで進める。t は前にだけ進められる。
  void advance(i64 t) {
    while (!pq.empty() && pq.top().t <= t) {
      Ev e = pq.top();
      pq.pop();
      if (e.g != gen[e.k] || e.t != evt[e.k]) continue;
      int old = win[e.k];
      pull(e.k, e.t), publish(e.k);
      if (win[e.k] == old) continue;  // 子の勝ち筋が変わらなければ上も変わらない
      for (int v = e.k >> 1; v >= 1; v >>= 1) {
        int ow = win[v];
        pull(v, e.t), publish(v);
        if (win[v] == ow) break;
      }
    }
    now = std::max(now, t);
  }

 private:
  i128 eval(int i, i64 t) const { return (i128)A[i] * t + B[i]; }
  bool better(int i, int j, i64 t) const {
    i128 vi = eval(i, t), vj = eval(j, t);
    return vi > vj || (vi == vj && i < j);
  }
  void pull(int k, i64 t) {
    int L = win[2 * k], R = win[2 * k + 1];
    if (L < 0) {
      win[k] = R, evt[k] = INF;
      return;
    }
    if (R < 0) {
      win[k] = L, evt[k] = INF;
      return;
    }
    int W = better(L, R, t) ? L : R, Z = W == L ? R : L;
    win[k] = W, evt[k] = INF;
    if (A[Z] > A[W]) {  // 敗者 Z の傾きが大きい。Z が厳密に勝つ最初の整数時刻
      i128 num = eval(W, t) - eval(Z, t);  // >= 0
      i128 den = (i128)A[Z] - A[W];
      i128 e = (i128)t + num / den + 1;
      evt[k] = e < (i128)INF ? (i64)e : INF;
    }
  }
  void publish(int k) {
    ++gen[k];
    if (evt[k] < INF) pq.push(Ev{evt[k], k, gen[k]});
  }
};
