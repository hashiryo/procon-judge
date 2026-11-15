#pragma once
#include "_mont.hpp"
#include "_pool.hpp"

// 触った所だけ節を作る二分木 (動的セグメント木) で、子の無い節は「受け持つ区間の値がすべて等しい」ことを表す。
// はじめは根だけで、値は 0。1 次関数は値の等しい区間を値の等しい区間へ移すので、作用が節の全体に掛かるなら
// 子を作らずに値を書き換えるだけで済む。子を作るのは、区間の端が節の途中にあって下りるときだけになる。
// 子のある節は、子へ配っていない作用 (b, c) を持つ。節は配列に置いて子を添字で持ち、値は Montgomery (2p 未満)。
struct Solver {
  static constexpr int H = 30;
  struct Node {
    u32 sum;     // 区間の和
    u32 x, y;    // 子があれば配っていない作用 v -> x v + y、無ければ x が区間の値
    u32 ch0, ch1;
  };
  PoolVec<Node> t;
  u32 pw[H + 1];  // 2^h

  explicit Solver(int) {
    t.reserve(1 << 12);
    t.push_back({0, 0, 0, 0, 0});  // 根。値はすべて 0
    for (int h = 0; h <= H; ++h) pw[h] = mont::enc(u64(1) << h);
  }

  static bool is_id(const Node &n) { return n.x == mont::R1 && n.y == 0; }

  // 長さ 2^h の節 v の全体に v -> b v + c を当てる。
  void all_apply(u32 v, int h, u32 b, u32 c) {
    Node &n = t[v];
    n.sum = mont::mulsum(b, n.sum, c, pw[h]);
    if (!n.ch0) n.x = mont::mulsum(b, n.x, c, mont::R1);
    else n.x = mont::mul(b, n.x), n.y = mont::mulsum(b, n.y, c, mont::R1);
  }

  // 下りる前に、値の等しい節は子を 2 つ作って分け、子のある節は作用を配る。
  void open(u32 v, int h) {
    if (!t[v].ch0) {
      const u32 val = t[v].x, s = mont::mul(val, pw[h - 1]), k = u32(t.size());
      t.push_back({s, val, 0, 0, 0});
      t.push_back({s, val, 0, 0, 0});
      Node &n = t[v];
      n.ch0 = k, n.ch1 = k + 1, n.x = mont::R1, n.y = 0;
    } else if (!is_id(t[v])) {
      const u32 b = t[v].x, c = t[v].y;
      all_apply(t[v].ch0, h - 1, b, c), all_apply(t[v].ch1, h - 1, b, c);
      t[v].x = mont::R1, t[v].y = 0;
    }
  }

  void apply_rec(u32 v, u32 lo, int h, u32 l, u32 r, u32 b, u32 c) {
    if (l <= lo && lo + (1u << h) <= r) return all_apply(v, h, b, c);
    open(v, h);
    const u32 mid = lo + (1u << (h - 1));
    if (l < mid) apply_rec(t[v].ch0, lo, h - 1, l, r, b, c);
    if (r > mid) apply_rec(t[v].ch1, mid, h - 1, l, r, b, c);
    t[v].sum = mont::add(t[t[v].ch0].sum, t[t[v].ch1].sum);
  }

  // 書き込みをせず、配っていない作用は戻りに当てる。
  u32 sum_rec(u32 v, u32 lo, int h, u32 l, u32 r) const {
    const Node &n = t[v];
    const u32 hi = lo + (1u << h);
    if (l <= lo && hi <= r) return n.sum;
    const u32 len = mont::enc(min(r, hi) - max(l, lo));
    if (!n.ch0) return mont::mul(n.x, len);
    const u32 mid = lo + (1u << (h - 1));
    u32 s = 0;
    if (l < mid) s = sum_rec(n.ch0, lo, h - 1, l, r);
    if (r > mid) s = mont::add(s, sum_rec(n.ch1, mid, h - 1, l, r));
    return mont::mulsum(n.x, s, n.y, len);
  }

  void affine(int l, int r, i64 b, i64 c) {
    // 1 回で作る節は、両端の道で段ごとに 2 個ずつまで。先に場所を取っておけば、下りる途中で配列が動かない。
    if (t.capacity() < t.size() + 4 * H) t.reserve(2 * t.capacity());
    apply_rec(0, 0, H, l, r, mont::enc(b), mont::enc(c));
  }

  i64 sum(int l, int r) { return mont::dec(sum_rec(0, 0, H, l, r)); }
};
