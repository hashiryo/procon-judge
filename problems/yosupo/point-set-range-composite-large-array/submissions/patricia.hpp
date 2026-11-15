#pragma once
#include "_mont.hpp"
#include "_pool.hpp"

// パトリシア木 (枝分かれの無い道を 1 つの節に畳んだ二分木)。節は [lo, lo + 2^lv) を受け持ち、葉は lv = 0。
// 内部の節は必ず子を 2 つ持つので、節の数は set した添字の数の 2 倍未満で、深さは添字の散らばり方で決まる。
// 節は配列に置いて子を添字で持つ。値は 1 次関数 x -> a x + b で、a と b は Montgomery (2p 未満)。
//
// 区間の取得は dyn と同じく、l と r - 1 が同じ子にある間は根から下り、分かれた節から先は左の道で l 以上の側、
// 右の道で r - 1 以下の側を集める。節の範囲が畳まれているので、範囲の全部が入るかは節の lo と lv で見る。
struct Solver {
  struct Node {
    u32 lo, lv, ch[2], a, b;
  };
  PoolVec<Node> t;
  u32 root = 0;

  explicit Solver(int) {
    t.reserve(1 << 12);
    t.push_back({0, 0, {0, 0}, mont::R1, 0});  // 0 は無い節
  }

  u32 apply(u32 v, u32 x) const { return mont::mulsum(t[v].a, x, t[v].b, mont::R1); }

  void pull(u32 v) {
    const Node &f = t[t[v].ch[0]], &g = t[t[v].ch[1]];
    t[v].a = mont::mul(g.a, f.a);
    t[v].b = mont::mulsum(g.a, f.b, g.b, mont::R1);
  }

  void set(int p_, i64 c, i64 d) {
    const u32 p = p_, A = mont::enc(c), B = mont::enc(d);
    // 1 回で作る節は 2 個以下。先に場所を取っておけば、下りる途中で配列が動かない。
    if (t.capacity() < t.size() + 2) t.reserve(2 * t.capacity());
    if (!root) {
      root = u32(t.size());
      t.push_back({p, 0, {0, 0}, A, B});
      return;
    }
    u32 path[32];
    int np = 0;
    u32 *slot = &root;
    for (;;) {
      const u32 v = *slot;
      Node &x = t[v];
      if ((p ^ x.lo) >> x.lv) {
        // p は v の範囲の外。v と p の葉を子に持つ節を v の場所に置く。
        const u32 lv = 32 - __builtin_clz(p ^ x.lo), bit = (p >> (lv - 1)) & 1;
        const u32 w = u32(t.size()), leaf = w + 1;
        Node nw{p & ~((1u << lv) - 1), lv, {0, 0}, 0, 0};
        nw.ch[bit] = leaf, nw.ch[bit ^ 1] = v;
        t.push_back(nw);
        t.push_back({p, 0, {0, 0}, A, B});
        *slot = w;
        pull(w);
        break;
      }
      if (x.lv == 0) {
        x.a = A, x.b = B;
        break;
      }
      path[np++] = v;
      slot = &x.ch[(p >> (x.lv - 1)) & 1];
    }
    while (np) pull(path[--np]);
  }

  i64 query(int l_, int r_, i64 x_) {
    const u32 l = l_, last = r_ - 1;
    u32 x = mont::enc(x_);
    // l と last が同じ子にある間は下りる。
    u32 v = root;
    for (;;) {
      if (!v) return x_;
      const Node &n = t[v];
      const u32 hi = n.lo + ((1u << n.lv) - 1);
      if (last < n.lo || hi < l) return x_;
      if (l <= n.lo && hi <= last) return mont::dec(apply(v, x));
      const u32 mid = n.lo + (1u << (n.lv - 1));
      if (last < mid) v = n.ch[0];
      else if (l >= mid) v = n.ch[1];
      else break;
    }
    // 左の子で l 以上の側。全部入る節を浅い方から積み、深い方から当てる。
    u32 st[32];
    int ns = 0;
    for (u32 u = t[v].ch[0];;) {
      const Node &n = t[u];
      if (n.lo >= l) {
        st[ns++] = u;
        break;
      }
      if (n.lo + ((1u << n.lv) - 1) < l) break;
      const u32 mid = n.lo + (1u << (n.lv - 1));
      if (l < mid) st[ns++] = n.ch[1], u = n.ch[0];
      else u = n.ch[1];
    }
    while (ns) x = apply(st[--ns], x);
    // 右の子で last 以下の側。左から右の順に出会うので、そのまま当てる。
    for (u32 u = t[v].ch[1];;) {
      const Node &n = t[u];
      if (n.lo + ((1u << n.lv) - 1) <= last) {
        x = apply(u, x);
        break;
      }
      if (n.lo > last) break;
      const u32 mid = n.lo + (1u << (n.lv - 1));
      if (last >= mid) x = apply(n.ch[0], x), u = n.ch[1];
      else u = n.ch[0];
    }
    return mont::dec(x);
  }
};
