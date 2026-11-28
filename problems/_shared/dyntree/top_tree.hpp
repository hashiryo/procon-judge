#pragma once
// 自己調整 top tree (nandhagk の 337329 の形)。部分木への加算と部分木和、道への加算と道の和を O(log N) で扱う。
// 値は long long、作用は加算 (長さを掛けて足す)。
// 節は道の splay (l, r) と rake の splay (h) の 2 種類を 1 つの型で持ち、p は両方の親を指す。
//   len, sum: 道の頂点数と和。cap, sub: 部分木の頂点数と和 (h の道も含む)。
//   lzy: 道への遅延、hzy: 部分木への遅延。
// expose(x) で x を根のクラスタにし、x->h が x の子の部分木 (rake) になる。
// 実装は nandhagk の lazy_top_tree を、加算専用にして deque の節に置き換えたもの。
#include <cassert>
#include <cstdint>
#include <deque>
#include <utility>
#include <vector>

class LazyTopTree {
 public:
  using i64 = long long;
  using u32 = unsigned;

  explicit LazyTopTree(const std::vector<i64> &a) : n((u32)a.size()) {
    buf.resize(n);
    for (u32 i = 0; i < n; ++i) buf[i] = Node(a[i]);
  }

  // 辺 (u, v) を張る。u と v は別の木にあること。
  void link(int u, int v) { link_(&buf[u], &buf[v]); }
  // 辺 (u, v) を切る。
  void cut(int u, int v) {
    evert_(&buf[v]);
    cut_(&buf[u]);
  }
  // 辺 (v, p) について、p を親とした v の部分木に f を足す。
  void apply_subtree(int v, int p, i64 f) {
    evert_(&buf[p]);
    apply_subtree_(&buf[v], f);
  }
  // 辺 (v, p) について、p を親とした v の部分木の和。
  i64 prod_subtree(int v, int p) {
    evert_(&buf[p]);
    return fold_subtree_(&buf[v]);
  }
  void set(int v, i64 x) {
    Node *t = &buf[v];
    evert_(t), expose_(t);
    t->val = x;
    pull<false>(t);
  }
  i64 get(int v) {
    Node *t = &buf[v];
    evert_(t), expose_(t);
    return t->val;
  }

 private:
  struct Node {
    Node *l = nullptr, *r = nullptr, *h = nullptr, *p = nullptr;
    i64 val = 0, sum = 0, sub = 0;
    i64 lzy = 0, hzy = 0;
    u32 len = 0, cap = 0;
    bool rev = false;
    Node() = default;
    explicit Node(i64 x) : val(x), sum(x), len(1) {}
    bool is_root() const { return !p || (p->l != this && p->r != this); }
  };

  std::deque<Node> buf;
  std::vector<Node *> fre;
  u32 n;

  static u32 len_(const Node *x) { return x ? x->len : 0; }
  static i64 sum_(const Node *x) { return x ? x->sum : 0; }
  static u32 cap_(const Node *x) { return x ? x->cap : 0; }
  static i64 sub_(const Node *x) { return x ? x->sub : 0; }

  static void setl(Node *x, Node *y) {
    if ((x->l = y)) y->p = x;
  }
  static void setr(Node *x, Node *y) {
    if ((x->r = y)) y->p = x;
  }
  static void seth(Node *x, Node *y) {
    if ((x->h = y)) y->p = x;
  }

  template <bool F>
  static void pull(Node *x) {
    if (!x) return;
    if constexpr (!F) {
      x->len = len_(x->l) + 1 + len_(x->r);
      x->sum = sum_(x->l) + x->val + sum_(x->r);
      x->cap = cap_(x->l) + cap_(x->r) + cap_(x->h);
      x->sub = sub_(x->l) + sub_(x->r) + sub_(x->h);
    } else {
      x->cap = cap_(x->l) + cap_(x->r) + cap_(x->h) + len_(x->h);
      x->sub = sub_(x->l) + sub_(x->r) + sub_(x->h) + sum_(x->h);
    }
  }

  static void apply_at(Node *x, i64 f) {
    x->lzy += f;
    x->val += f;
    x->sum += f * (i64)x->len;
  }
  static void apply_sub(Node *x, i64 f) {
    x->hzy += f;
    x->sub += f * (i64)x->cap;
  }
  static void flip(Node *x) {
    std::swap(x->l, x->r);
    x->rev ^= true;
  }
  template <bool F>
  static void push(Node *x) {
    if (!x) return;
    if constexpr (!F) {
      if (x->rev) {
        if (x->l) flip(x->l);
        if (x->r) flip(x->r);
        x->rev = false;
      }
      if (x->lzy) {
        if (x->l) apply_at(x->l, x->lzy);
        if (x->r) apply_at(x->r, x->lzy);
        x->lzy = 0;
      }
      if (x->hzy) {
        if (x->l) apply_sub(x->l, x->hzy);
        if (x->r) apply_sub(x->r, x->hzy);
        if (x->h) apply_sub(x->h, x->hzy);
        x->hzy = 0;
      }
    } else {
      if (x->hzy) {
        if (x->l) apply_sub(x->l, x->hzy);
        if (x->r) apply_sub(x->r, x->hzy);
        if (x->h) {
          apply_sub(x->h, x->hzy);
          apply_at(x->h, x->hzy);
        }
        x->hzy = 0;
      }
    }
  }

  template <bool F>
  static void rotate(Node *x) {
    Node *p = x->p, *g = p->p;
    if (g) {
      if (g->h == p) g->h = x;
      else if (g->r == p) g->r = x;
      else g->l = x;
    }
    if (p->r == x) {
      setr(p, x->l);
      setl(x, p);
    } else {
      setl(p, x->r);
      setr(x, p);
    }
    x->p = g;
    pull<F>(p), pull<F>(x);
  }

  template <bool F>
  static void down(Node *x) {
    if (!x->is_root()) down<F>(x->p);
    push<F>(x);
  }
  template <bool F>
  static void splay(Node *x) {
    down<F>(x);
    while (!x->is_root()) {
      Node *p = x->p, *q = p->p;
      if (!p->is_root()) rotate<F>(((p == q->l) ^ (x == p->l)) ? x : p);
      rotate<F>(x);
    }
  }
  template <bool F>
  static void down(Node *x, Node *g) {
    if (!x->is_root() && x->p != g) down<F>(x->p, g);
    push<F>(x);
  }
  template <bool F>
  static void splay(Node *x, Node *g) {
    down<F>(x, g);
    while (!x->is_root() && x->p != g) {
      Node *p = x->p, *q = p->p;
      if (!p->is_root() && q != g) rotate<F>(((p == q->l) ^ (x == p->l)) ? x : p);
      rotate<F>(x);
    }
  }
  static void splay_rightmost(Node *x) {
    Node *g = x->p;
    for (; x->r; x = x->r) push<true>(x);
    splay<true>(x, g);
  }

  Node *create() {
    Node *v;
    if (fre.empty()) {
      buf.emplace_back();
      v = &buf.back();
    } else {
      v = fre.back();
      fre.pop_back();
    }
    v->len = 1;
    return v;
  }
  void del(Node *x) {
    fre.push_back(x);
    *x = Node();
  }

  void splice(Node *x) {
    splay<true>(x), x = x->p, splay<false>(x);
    Node *y = x->h;
    push<true>(y);
    if (x->r) {
      std::swap(x->r->p, y->h->p);
      std::swap(x->r, y->h);
    } else {
      setr(x, y->h);
      if (y->l) {
        splay_rightmost(y->l);
        setr(y->l, y->r);
        x->h = y->l;
      } else {
        push<true>(y->r);
        x->h = y->r;
      }
      del(y);
      if ((y = x->h)) y->p = x;
    }
    pull<true>(y), pull<false>(x);
    push<false>(x->r);
    rotate<false>(x->r);
  }

  void expose_(Node *x) {
    splay<false>(x);
    if (x->r) {
      Node *y = create();
      setl(y, x->h);
      seth(y, x->r);
      pull<true>(y);
      x->r = nullptr;
      seth(x, y);
      pull<false>(x);
    }
    while (x->p) splice(x->p);
  }

  void evert_(Node *x) {
    expose_(x);
    flip(x);
  }
  void link_(Node *x, Node *y) {
    evert_(y), expose_(x);
    setr(x, y);
    pull<false>(x);
  }
  void cut_(Node *x) {
    expose_(x);
    push<false>(x);
    if (x->l) {
      x->l->p = nullptr;
      x->l = nullptr;
      pull<false>(x);
    }
  }
  void apply_subtree_(Node *x, i64 f) {
    expose_(x);
    x->val += f;
    pull<false>(x);
    if (x->h) apply_sub(x->h, f);
  }
  i64 fold_subtree_(Node *x) {
    expose_(x);
    return x->val + sub_(x->h);
  }
};
