// 円形クエリ。点を葉に置く完全 2 分木の KD 木 (子は 2v と 2v+1) で、円に入る点を数える。
// 節は外接矩形 (半開) と部分木の点の数を持ち、円との距離の下限が半径を超える節を打ち切り、
// 上限が半径以下ならその節の数をまとめて足す。lib.cpp (今の Library の KDTree::prod_ball) と比べる。
#include <algorithm>
#include <array>
#include <iostream>
#include <numeric>
#include <vector>
using namespace std;

struct KdBall {
  using i64 = long long;
  struct Node {
    int lx, ly, hx, hy;  // 外接矩形 (半開)
    int cnt;             // 部分木の点の数
    int lo, hi;          // 葉なら点の範囲 [lo, hi)。内部なら lo = -1
  };

  vector<Node> t;
  vector<int> bx, by;
  int n_ = 0;

  explicit KdBall(const vector<array<int, 2>> &pts) {
    const int n = n_ = (int)pts.size();
    t.assign((size_t)4 * n + 4, Node{0, 0, 0, 0, 0, -1, -1});
    bx.resize(n);
    by.resize(n);
    if (n == 0) return;
    vector<int> idx(n);
    iota(idx.begin(), idx.end(), 0);
    build(pts, idx, 1, 0, n, 0);
  }

  // 中心 (x, y)、半径の 2 乗 r2 の円に入る点の数。
  i64 count(int x, int y, i64 r2) const {
    if (n_ == 0) return 0;
    i64 ret = 0;
    int st[128];
    int sp = 0;
    st[sp++] = 1;
    while (sp) {
      const int v = st[--sp];
      const Node &p = t[v];
      if (min_dist2(p, x, y) > r2) continue;
      if (max_dist2(p, x, y) <= r2) {
        ret += p.cnt;
        continue;
      }
      if (p.lo >= 0) {
        for (int j = p.lo; j < p.hi; ++j) {
          const i64 dx = (i64)bx[j] - x, dy = (i64)by[j] - y;
          if (dx * dx + dy * dy <= r2) ++ret;
        }
        continue;
      }
      st[sp++] = 2 * v;
      st[sp++] = 2 * v + 1;
    }
    return ret;
  }

 private:
  static i64 clamp_dist(i64 a, int lo, int hi) {  // [lo, hi) の点のうち a に一番近いものとの距離
    if (a < lo) return lo - a;
    if (a > hi - 1) return a - (hi - 1);
    return 0;
  }
  static i64 min_dist2(const Node &p, int x, int y) {
    const i64 dx = clamp_dist(x, p.lx, p.hx), dy = clamp_dist(y, p.ly, p.hy);
    return dx * dx + dy * dy;
  }
  static i64 max_dist2(const Node &p, int x, int y) {
    const i64 dx = max(abs((i64)p.lx - x), abs((i64)p.hx - 1 - x));
    const i64 dy = max(abs((i64)p.ly - y), abs((i64)p.hy - 1 - y));
    return dx * dx + dy * dy;
  }

  void build(const vector<array<int, 2>> &pts, vector<int> &idx, int v, int l, int r, int dep) {
    if (r - l == 1) {
      const int i = idx[l];
      bx[l] = pts[i][0];
      by[l] = pts[i][1];
      t[v] = Node{bx[l], by[l], bx[l] + 1, by[l] + 1, 1, l, r};
      return;
    }
    const int mid = (l + r) >> 1;
    if ((dep & 1) == 0)
      nth_element(idx.begin() + l, idx.begin() + mid, idx.begin() + r,
                  [&pts](int a, int b) { return pts[a][0] < pts[b][0]; });
    else
      nth_element(idx.begin() + l, idx.begin() + mid, idx.begin() + r,
                  [&pts](int a, int b) { return pts[a][1] < pts[b][1]; });
    build(pts, idx, 2 * v, l, mid, dep + 1);
    build(pts, idx, 2 * v + 1, mid, r, dep + 1);
    Node &p = t[v];
    const Node &lc = t[2 * v], &rc = t[2 * v + 1];
    p.lx = min(lc.lx, rc.lx);
    p.ly = min(lc.ly, rc.ly);
    p.hx = max(lc.hx, rc.hx);
    p.hy = max(lc.hy, rc.hy);
    p.cnt = lc.cnt + rc.cnt;
  }
};

signed main() {
  cin.tie(0);
  ios::sync_with_stdio(0);
  for (int AN, BN, R; cin >> AN >> BN >> R && AN;) {
    vector<array<int, 2>> A(AN), B(BN);
    for (int i = 0; i < AN; ++i) cin >> A[i][0] >> A[i][1];
    for (int i = 0; i < BN; ++i) cin >> B[i][0] >> B[i][1];
    KdBall kd(B);
    long long ans = 0;
    for (auto [x, y] : A) ans += kd.count(x, y, (long long)R * 4 * R * 4);
    cout << ans << '\n';
  }
  return 0;
}
