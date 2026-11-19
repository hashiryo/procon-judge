#pragma once
// 直線 y = a x + b の集合の、点 x での最小を問う Li Chao 木の候補 2 つ。どちらも最小で、最大は呼ぶ側が a、b、答えの符号を
// 反転する。値は i64 で、a x + b が i64 に収まること。直線が 1 本も載っていない点の答えは LICHAO_INF。
//
// LiChaoRange: x の範囲 [lo, hi) を渡し、節を使う所だけ作る形。範囲は 2 の冪に切り上げ、節は配列の添字で持つ。直線を足すと
//   根から 1 本の道を下り、節が無い所に着いたらそこに置く。線分 [l, r) は、範囲を覆う節に分けてから、それぞれの節の下で直線を
//   足すのと同じように下る。問いは根から 1 本の道を下り、通った節の直線の最小を取る。
// LiChaoPoints: 問いの x を全部先に渡す形。x を並べて重複を除き、葉の数を 2 の冪に切り上げた完全二分木を配列に置く。直線を
//   足すときは、節の両端で新しい直線が勝つか負けるかを見て、両端とも負けたらそこで止める。問いは葉から根へ上り、通る節の添字が
//   前の節の結果を待たない。線分は葉の添字の区間 [l, r) で渡す。
#include <algorithm>
#include <climits>
#include <cstdint>
#include <utility>
#include <vector>

constexpr long long LICHAO_INF = LLONG_MAX;

struct LiChaoRange {
  struct Node {
    long long a, b;
    int c[2];
  };
  long long lo;
  int depth;  // 根が覆う幅は 2^depth
  int root = -1;
  std::vector<Node> t;

  LiChaoRange(long long lo, long long hi, int reserve_nodes = 0) : lo(lo), depth(0) {
    while ((1LL << depth) < hi - lo) ++depth;
    t.reserve(reserve_nodes);
  }

  int make(long long a, long long b) {
    t.push_back({a, b, {-1, -1}});
    return (int)t.size() - 1;
  }

  // 節 k (左端 l、幅 2^d) の下に直線を足す。
  void descend(int k, long long l, int d, long long a, long long b) {
    for (;;) {
      Node &nd = t[k];
      if (d == 0) {
        const long long x = lo + l;
        if (a * x + b < nd.a * x + nd.b) nd.a = a, nd.b = b;
        return;
      }
      const long long xl = lo + l, m = l + (1LL << (d - 1)), xm = lo + m;
      const bool bl = a * xl + b < nd.a * xl + nd.b;
      const bool bm = a * xm + b < nd.a * xm + nd.b;
      if (bm) std::swap(nd.a, a), std::swap(nd.b, b);
      const int dir = bl == bm;  // 負けた直線が左端で勝つなら左 (0)、そうでなければ右 (1)
      if (dir) l = m;
      --d;
      const int c = nd.c[dir];
      if (c < 0) {
        const int nk = make(a, b);  // t が伸びると nd は無効になるので、添字で書き直す
        t[k].c[dir] = nk;
        return;
      }
      k = c;
    }
  }

  void add_line(long long a, long long b) {
    if (root < 0) {
      root = make(a, b);
      return;
    }
    descend(root, 0, depth, a, b);
  }

  // 線分 a x + b (x in [sl, sr))。
  void add_segment(long long sl, long long sr, long long a, long long b) {
    sl = std::max(sl - lo, 0LL), sr = std::min(sr - lo, 1LL << depth);
    if (sl >= sr) return;
    if (root < 0) root = make(0, LICHAO_INF);
    seg(root, 0, depth, sl, sr, a, b);
  }

  void seg(int k, long long l, int d, long long sl, long long sr, long long a, long long b) {
    const long long r = l + (1LL << d);
    if (sl <= l && r <= sr) return descend(k, l, d, a, b);
    const long long m = l + (1LL << (d - 1));
    for (int dir = 0; dir < 2; ++dir) {
      const long long cl = dir ? m : l, cr = dir ? r : m;
      if (cr <= sl || sr <= cl) continue;
      int c = t[k].c[dir];
      if (c < 0) {
        c = make(0, LICHAO_INF);
        t[k].c[dir] = c;
      }
      seg(c, cl, d - 1, sl, sr, a, b);
    }
  }

  long long query(long long x) const {
    long long res = LICHAO_INF;
    const long long off = x - lo;
    int k = root, d = depth;
    while (k >= 0) {
      const Node &nd = t[k];
      res = std::min(res, nd.a * x + nd.b);  // 直線の無い節は (0, LICHAO_INF) なのであふれない
      if (d == 0) break;
      --d;
      k = nd.c[off >> d & 1];
    }
    return res;
  }
};

struct LiChaoPoints {
  struct Line {
    long long a, b;
  };
  int n = 1;  // 葉の数 (2 の冪)
  std::vector<long long> xs;  // 長さ n。重複を除いて並べ、末尾を最後の値で埋める
  int m = 0;                  // 重複を除いた点の数
  std::vector<Line> t;        // 節 1..2n-1。葉は n..2n-1

  explicit LiChaoPoints(std::vector<long long> pts) {
    std::sort(pts.begin(), pts.end());
    pts.erase(std::unique(pts.begin(), pts.end()), pts.end());
    init(std::move(pts));
  }

  struct SortedUnique {};
  // 並べて重複を除いた点を受ける。呼ぶ側が座標を詰めて添字を持つ使い方のため。
  LiChaoPoints(std::vector<long long> pts, SortedUnique) { init(std::move(pts)); }

  void init(std::vector<long long> pts) {
    m = (int)pts.size();
    while (n < m) n <<= 1;
    xs = std::move(pts);
    xs.resize(n, m ? xs.back() : 0);
    t.assign(2 * n, Line{0, LICHAO_INF});
  }

  static long long f(const Line &e, long long x) { return e.a * x + e.b; }  // 直線の無い節は (0, LICHAO_INF)

  // 節 k (葉の添字の区間 [l, r)) の下に直線を足す。
  void descend(int k, int l, int r, long long a, long long b) {
    for (;;) {
      Line &e = t[k];
      const long long xl = xs[l], xr = xs[r - 1];
      const bool bl = a * xl + b < f(e, xl), br = a * xr + b < f(e, xr);
      if (bl == br) {
        if (bl) e = {a, b};
        return;
      }
      const int mid = (l + r) >> 1;
      const long long xm = xs[mid];
      const bool bm = a * xm + b < f(e, xm);
      if (bm) std::swap(e.a, a), std::swap(e.b, b);
      // 負けた直線が勝つのは、左端で勝つなら [l, mid) の中、右端で勝つなら [mid, r) の中。
      if (bl != bm) k = 2 * k, r = mid;
      else k = 2 * k + 1, l = mid;
    }
  }

  void add_line(long long a, long long b) { descend(1, 0, n, a, b); }

  // 葉の添字の区間 [l, r) にだけ載る線分。
  void add_segment_index(int l, int r, long long a, long long b) {
    if (l >= r) return;
    int lk = l + n, rk = r + n, w = 1;
    for (; lk < rk; lk >>= 1, rk >>= 1, w <<= 1) {
      if (lk & 1) descend(lk, (lk * w) - n, (lk * w) - n + w, a, b), ++lk;
      if (rk & 1) --rk, descend(rk, (rk * w) - n, (rk * w) - n + w, a, b);
    }
  }

  // x 座標の区間 [xl, xr) にだけ載る線分。
  void add_segment(long long xl, long long xr, long long a, long long b) {
    const int l = (int)(std::lower_bound(xs.begin(), xs.begin() + m, xl) - xs.begin());
    const int r = (int)(std::lower_bound(xs.begin(), xs.begin() + m, xr) - xs.begin());
    add_segment_index(l, r, a, b);
  }

  int index(long long x) const { return (int)(std::lower_bound(xs.begin(), xs.begin() + m, x) - xs.begin()); }

  // 先に渡した点の添字 i での最小。
  long long query_index(int i) const {
    const long long x = xs[i];
    long long res = LICHAO_INF;
    for (int k = i + n; k >= 1; k >>= 1) res = std::min(res, f(t[k], x));
    return res;
  }

  long long query(long long x) const { return query_index(index(x)); }
};
