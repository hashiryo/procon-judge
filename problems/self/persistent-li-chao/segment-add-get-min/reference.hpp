#pragma once
// 期待出力を作る参照実装。永続 Li Chao 木を NeoLibrary を使わずに素直に書いたもの。
// 愚直解 (brute.hpp) と小さい入力で突き合わせる (pj testdata crosscheck)。
#include <algorithm>
#include <climits>
#include <vector>

struct Solver {
  static constexpr long long LO = -1000000, HI = 1000001;
  static constexpr long long INF = LLONG_MAX;
  struct Node {
    long long a, b;
    int c[2];
    bool has;
  };
  std::vector<Node> t;
  std::vector<int> ver;
  int depth = 0;
  explicit Solver(int q) {
    while ((1LL << depth) < HI - LO) ++depth;
    t.reserve(std::size_t(q) * 500 + 1);
    t.push_back(Node{0, INF, {-1, -1}, false});
    ver.reserve(q + 1);
    ver.push_back(-1);
  }
  static bool wins(long long u, long long v) { return u < v; }
  long long eval(const Node &n, long long x) const { return n.a * x + n.b; }
  int make(long long a, long long b, bool has) {
    t.push_back(Node{a, b, {-1, -1}, has});
    return int(t.size()) - 1;
  }
  int copy(int p) {
    t.push_back(t[p]);
    return int(t.size()) - 1;
  }
  void descend(int q, long long l, int d, long long a, long long b) {
    for (;;) {
      if (!t[q].has) {
        t[q].a = a, t[q].b = b, t[q].has = true;
        return;
      }
      const long long xl = LO + l, xr = std::min(LO + l + (1LL << d) - 1, HI - 1);
      const bool bl = wins(a * xl + b, eval(t[q], xl)), br = wins(a * xr + b, eval(t[q], xr));
      if (bl == br) {
        if (bl) t[q].a = a, t[q].b = b;
        return;
      }
      const long long m = l + (1LL << (d - 1));
      const bool bm = wins(a * (LO + m) + b, eval(t[q], LO + m));
      if (bm) std::swap(t[q].a, a), std::swap(t[q].b, b);
      const int dir = bl == bm;
      if (dir) l = m;
      --d;
      const int c = t[q].c[dir];
      if (c < 0) {
        t[q].c[dir] = make(a, b, true);
        return;
      }
      const int nc = copy(c);
      t[q].c[dir] = nc;
      q = nc;
    }
  }
  void cover(int q, long long l, int d, long long sl, long long sr, long long a, long long b) {
    const long long r = l + (1LL << d);
    if (sl <= l && r <= sr) return descend(q, l, d, a, b);
    const long long m = l + (1LL << (d - 1));
    for (int dir = 0; dir < 2; ++dir) {
      const long long cl = dir ? m : l, cr = dir ? r : m;
      if (cr <= sl || sr <= cl) continue;
      const int c = t[q].c[dir];
      const int nc = c < 0 ? make(a, b, false) : copy(c);
      t[q].c[dir] = nc;
      cover(nc, cl, d - 1, sl, sr, a, b);
    }
  }
  void insert(int v, long long l, long long r, long long a, long long b) {
    const long long sl = std::max(l - LO, 0LL), sr = std::min(r - LO, 1LL << depth);
    const int root = ver[v];
    if (sl >= sr) {
      ver.push_back(root);
      return;
    }
    const int nroot = root < 0 ? make(a, b, false) : copy(root);
    cover(nroot, 0, depth, sl, sr, a, b);
    ver.push_back(nroot);
  }
  long long query(int v, long long x) const {
    const long long off = x - LO;
    long long res = INF;
    int k = ver[v], d = depth;
    while (k >= 0) {
      if (t[k].has) res = std::min(res, t[k].a * x + t[k].b);
      if (d == 0) break;
      --d, k = t[k].c[off >> d & 1];
    }
    return res;
  }
};
