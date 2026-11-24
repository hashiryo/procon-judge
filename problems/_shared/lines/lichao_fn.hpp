#pragma once
// lichao.hpp の 2 つの Li Chao 木を、載せる関数の型 F をテンプレートで受ける形にしたもの。F は x (long long) を受けて値を返す
// operator() を持ち、どの 2 つも高々 1 回しか交わらない族 (擬直線) であること。直線は LineFn。最小を求める。
// 関数の載っていない節は、節ごとの印 has で見分ける (lichao.hpp は傾き 0、切片 LICHAO_INF の直線で表していた)。関数が 1 つも
// 載っていない点の答えは、値の型の最大値。
//
// LiChaoRangeFn<F>: x の範囲 [lo, hi) を渡し、節を使う所だけ作る形。
// LiChaoPointsFn<F>: 問いの x を全部先に渡し、座標を詰めた完全二分木を配列に置く形。問いは葉から根へ上る。
// 追加はどちらも、節の両端で新しい関数が両方勝てば置き換え、両方負ければ止める。交わりが高々 1 回なので、両端で勝てば
// 節の区間の全体で勝つ。
#include <algorithm>
#include <limits>
#include <type_traits>
#include <utility>
#include <vector>

struct LineFn {
  long long a, b;
  long long operator()(long long x) const { return a * x + b; }
};

template <class F = LineFn> struct LiChaoRangeFn {
  using V = std::invoke_result_t<const F &, long long>;
  static constexpr V INF = std::numeric_limits<V>::max();
  struct Node {
    F f;
    int c[2];
    bool has;
  };
  long long lo;
  int depth;  // 根が覆う幅は 2^depth
  int root = -1;
  std::vector<Node> t;

  LiChaoRangeFn(long long lo, long long hi, int reserve_nodes = 0) : lo(lo), depth(0) {
    while ((1LL << depth) < hi - lo) ++depth;
    t.reserve(reserve_nodes);
  }

  int make(const F &f, bool has) {
    t.push_back({f, {-1, -1}, has});
    return (int)t.size() - 1;
  }

  // 節 k (左端 l、幅 2^d) の下に関数を足す。
  void descend(int k, long long l, int d, F f) {
    for (;;) {
      Node &nd = t[k];
      if (!nd.has) {
        nd.f = f, nd.has = true;
        return;
      }
      const long long xl = lo + l, xr = xl + (1LL << d) - 1;
      const bool bl = f(xl) < nd.f(xl), br = f(xr) < nd.f(xr);
      if (bl == br) {
        if (bl) nd.f = f;
        return;
      }
      const long long m = l + (1LL << (d - 1)), xm = lo + m;
      const bool bm = f(xm) < nd.f(xm);
      if (bm) std::swap(nd.f, f);
      const int dir = bl == bm;  // 負けた関数が左端で勝つなら左 (0)、そうでなければ右 (1)
      if (dir) l = m;
      --d;
      const int c = nd.c[dir];
      if (c < 0) {
        const int nk = make(f, true);  // t が伸びると nd は無効になるので、添字で書き直す
        t[k].c[dir] = nk;
        return;
      }
      k = c;
    }
  }

  void add(const F &f) {
    if (root < 0) {
      root = make(f, true);
      return;
    }
    descend(root, 0, depth, f);
  }

  // x in [sl, sr) にだけ載せる。
  void add(const F &f, long long sl, long long sr) {
    sl = std::max(sl - lo, 0LL), sr = std::min(sr - lo, 1LL << depth);
    if (sl >= sr) return;
    if (root < 0) root = make(f, false);
    seg(root, 0, depth, sl, sr, f);
  }

  void seg(int k, long long l, int d, long long sl, long long sr, const F &f) {
    const long long r = l + (1LL << d);
    if (sl <= l && r <= sr) return descend(k, l, d, f);
    const long long m = l + (1LL << (d - 1));
    for (int dir = 0; dir < 2; ++dir) {
      const long long cl = dir ? m : l, cr = dir ? r : m;
      if (cr <= sl || sr <= cl) continue;
      int c = t[k].c[dir];
      if (c < 0) {
        c = make(f, false);
        t[k].c[dir] = c;
      }
      seg(c, cl, d - 1, sl, sr, f);
    }
  }

  V query(long long x) const {
    V res = INF;
    const long long off = x - lo;
    int k = root, d = depth;
    while (k >= 0) {
      const Node &nd = t[k];
      if (nd.has) res = std::min(res, nd.f(x));
      if (d == 0) break;
      --d;
      k = nd.c[off >> d & 1];
    }
    return res;
  }
};

template <class F = LineFn> struct LiChaoPointsFn {
  using V = std::invoke_result_t<const F &, long long>;
  static constexpr V INF = std::numeric_limits<V>::max();
  struct Slot {
    F f;
    bool has;
  };
  int n = 1;                  // 葉の数 (2 の冪)
  std::vector<long long> xs;  // 長さ n。重複を除いて並べ、末尾を最後の値で埋める
  int m = 0;                  // 重複を除いた点の数
  std::vector<Slot> t;        // 節 1..2n-1。葉は n..2n-1

  explicit LiChaoPointsFn(std::vector<long long> pts) {
    std::sort(pts.begin(), pts.end());
    pts.erase(std::unique(pts.begin(), pts.end()), pts.end());
    init(std::move(pts));
  }

  struct SortedUnique {};
  LiChaoPointsFn(std::vector<long long> pts, SortedUnique) { init(std::move(pts)); }

  void init(std::vector<long long> pts) {
    m = (int)pts.size();
    while (n < m) n <<= 1;
    xs = std::move(pts);
    xs.resize(n, m ? xs.back() : 0);
    t.assign(2 * n, Slot{F{}, false});
  }

  // 節 k (葉の添字の区間 [l, r)) の下に関数を足す。
  void descend(int k, int l, int r, F f) {
    for (;;) {
      Slot &e = t[k];
      if (!e.has) {
        e.f = f, e.has = true;
        return;
      }
      const long long xl = xs[l], xr = xs[r - 1];
      const bool bl = f(xl) < e.f(xl), br = f(xr) < e.f(xr);
      if (bl == br) {
        if (bl) e.f = f;
        return;
      }
      const int mid = (l + r) >> 1;
      const long long xm = xs[mid];
      const bool bm = f(xm) < e.f(xm);
      if (bm) std::swap(e.f, f);
      if (bl != bm) k = 2 * k, r = mid;
      else k = 2 * k + 1, l = mid;
    }
  }

  void add(const F &f) { descend(1, 0, n, f); }

  // 葉の添字の区間 [l, r) にだけ載せる。
  void add_index(const F &f, int l, int r) {
    if (l >= r) return;
    int lk = l + n, rk = r + n, w = 1;
    for (; lk < rk; lk >>= 1, rk >>= 1, w <<= 1) {
      if (lk & 1) descend(lk, (lk * w) - n, (lk * w) - n + w, f), ++lk;
      if (rk & 1) --rk, descend(rk, (rk * w) - n, (rk * w) - n + w, f);
    }
  }

  int index(long long x) const { return (int)(std::lower_bound(xs.begin(), xs.begin() + m, x) - xs.begin()); }

  // x in [xl, xr) にだけ載せる。
  void add(const F &f, long long xl, long long xr) { add_index(f, index(xl), index(xr)); }

  // 先に渡した点の添字 i での最小。
  V query_index(int i) const {
    const long long x = xs[i];
    V res = INF;
    for (int k = i + n; k >= 1; k >>= 1)
      if (t[k].has) res = std::min(res, t[k].f(x));
    return res;
  }

  V query(long long x) const { return query_index(index(x)); }
};
