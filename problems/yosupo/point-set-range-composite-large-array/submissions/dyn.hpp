#pragma once
#include "_mont.hpp"
#include "_pool.hpp"

// 触った所だけ節を作る二分木 (動的セグメント木)。添字の空間は [0, 2^30) で、節は配列に置いて子を添字で持つ。
// 節 0 は無い子を表し、恒等写像の値を持たせておく。値の取り直しで子の有無を分けずに済む。
// 値は 1 次関数 x -> a x + b で、a と b は Montgomery (2p 未満)。
//
// 区間の取得は、l と r - 1 が同じ子にある間は根から下り、分かれた節から先は左の道で l 以上の側、右の道で
// r - 1 以下の側を集める。右の道は左から右の順に出会うので x にそのまま当て、左の道は深い方から当てる。
struct Solver {
  static constexpr int H = 30;
  struct Node {
    u32 ch[2], a, b;
  };
  PoolVec<Node> t;

  explicit Solver(int) {
    t.reserve(1 << 12);
    t.push_back({{0, 0}, mont::R1, 0});  // 無い子 (恒等写像)
    t.push_back({{0, 0}, mont::R1, 0});  // 根
  }

  // g(x) = a x + b を x に当てる。
  u32 apply(u32 v, u32 x) const { return mont::mulsum(t[v].a, x, t[v].b, mont::R1); }

  // f のあとに g を当てる合成を節 v に書く (f が左の子、g が右の子)。
  void pull(u32 v) {
    const Node &f = t[t[v].ch[0]], &g = t[t[v].ch[1]];
    t[v].a = mont::mul(g.a, f.a);
    t[v].b = mont::mulsum(g.a, f.b, g.b, mont::R1);
  }

  void set(int p, i64 c, i64 d) {
    // 1 回で作る節は H 個以下。先に場所を取っておけば、下りる途中で配列が動かない。
    if (t.capacity() < t.size() + H) t.reserve(2 * t.capacity());
    u32 path[H];
    u32 v = 1;
    for (int h = H - 1; h >= 0; --h) {
      path[h] = v;
      u32 &ch = t[v].ch[(p >> h) & 1];
      if (!ch) {
        ch = u32(t.size());
        t.push_back({{0, 0}, mont::R1, 0});
      }
      v = ch;
    }
    t[v].a = mont::enc(c), t[v].b = mont::enc(d);
    for (int h = 0; h < H; ++h) pull(path[h]);
  }

  i64 query(int l_, int r_, i64 x_) {
    const u32 l = l_, last = r_ - 1;
    u32 x = mont::enc(x_);
    // l と last が同じ子にある間は下りる。
    u32 v = 1;
    int h = H;
    for (; h > 0; --h) {
      const u32 bl = (l >> (h - 1)) & 1;
      if (bl != ((last >> (h - 1)) & 1)) break;
      v = t[v].ch[bl];
      if (!v) return x_;
    }
    if (h == 0) return mont::dec(apply(v, x));
    // 左の子で l 以上の側。全部入る節を浅い方から積み、深い方から当てる。
    u32 st[H];
    int ns = 0;
    for (u32 u = t[v].ch[0], k = h - 1; u; --k) {
      if ((l & ((1u << k) - 1)) == 0) {
        st[ns++] = u;
        break;
      }
      if ((l >> (k - 1)) & 1) u = t[u].ch[1];
      else st[ns++] = t[u].ch[1], u = t[u].ch[0];
    }
    while (ns) x = apply(st[--ns], x);
    // 右の子で last 以下の側。左から右の順に出会うので、そのまま当てる。
    for (u32 u = t[v].ch[1], k = h - 1; u; --k) {
      if (((last + 1) & ((1u << k) - 1)) == 0) {
        x = apply(u, x);
        break;
      }
      if ((last >> (k - 1)) & 1) x = apply(t[u].ch[0], x), u = t[u].ch[1];
      else u = t[u].ch[0];
    }
    return mont::dec(x);
  }
};
