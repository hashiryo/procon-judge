// 列挙の verify。点を葉に置く完全 2 分木の KD 木 (子は 2v と 2v+1) で、長方形に入る点を集める。
// 深さで x と y を交互に、中央値で分ける。各節は外接矩形 (半開) を持ち、問いの範囲と
// 交わらない節を打ち切る。lib.cpp (今の Library の KDTree::enum_cuboid) と比べるための提出。
#include <algorithm>
#include <array>
#include <iostream>
#include <numeric>
#include <vector>
using namespace std;

struct KdEnum {
  struct Node {
    int lx, ly, hx, hy;  // 外接矩形 (半開)
    int lo, hi;          // 葉なら点の範囲 [lo, hi)。内部なら lo = -1
  };

  vector<Node> t;
  vector<int> bx, by, bid;
  int n_ = 0;  // 葉の順に並べた点

  explicit KdEnum(const vector<array<int, 3>> &pts) {
    const int n = n_ = (int)pts.size();
    t.assign((size_t)4 * n + 4, Node{0, 0, 0, 0, -1, -1});
    vector<int> idx(n);
    iota(idx.begin(), idx.end(), 0);
    bx.resize(n);
    by.resize(n);
    bid.resize(n);
    if (n) build(pts, idx, 1, 0, n, 0);
  }

  // [l, r) の x と [d, u) の y に入る点の番号を out に積む。
  template <class F> void query(int l, int d, int r, int u, F out) const {
    if (n_ == 0) return;
    int st[128];
    int sp = 0;
    st[sp++] = 1;
    while (sp) {
      const int v = st[--sp];
      const Node &p = t[v];
      if (p.hx <= l || r <= p.lx || p.hy <= d || u <= p.ly) continue;
      if (p.lo >= 0) {
        for (int j = p.lo; j < p.hi; ++j)
          if (l <= bx[j] && bx[j] < r && d <= by[j] && by[j] < u) out(bid[j]);
        continue;
      }
      st[sp++] = 2 * v;
      st[sp++] = 2 * v + 1;
    }
  }

 private:
  void build(const vector<array<int, 3>> &pts, vector<int> &idx, int v, int l, int r, int dep) {
    if (r - l == 1) {
      const int i = idx[l];
      bx[l] = pts[i][0];
      by[l] = pts[i][1];
      bid[l] = pts[i][2];
      t[v] = Node{bx[l], by[l], bx[l] + 1, by[l] + 1, l, r};
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
  }
};

signed main() {
  cin.tie(0);
  ios::sync_with_stdio(0);
  int n;
  cin >> n;
  vector<array<int, 3>> xy(n);
  for (int i = 0; i < n; ++i) cin >> xy[i][0] >> xy[i][1], xy[i][2] = i;
  KdEnum kd(xy);
  int q;
  cin >> q;
  vector<int> ans;
  while (q--) {
    int sx, tx, sy, ty;
    cin >> sx >> tx >> sy >> ty;
    ans.clear();
    kd.query(sx, sy, tx + 1, ty + 1, [&ans](int id) { ans.push_back(id); });
    sort(ans.begin(), ans.end());
    for (auto x : ans) cout << x << '\n';
    cout << '\n';
  }
  return 0;
}
