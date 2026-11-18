#pragma once
#include "pj.hpp"
#include "neo/algebra/ZMod.hpp"

// 節が値の配列 (最大 B 個) を持つ treap。Library Checker の首位 (nandhagk の 402977、葉に配列を持つ B 木のような列) と同じく、
// 節の数を値の数の B 分の 1 ほどにして、木をたどる分をキャッシュに収める。節の情報と値の配列は別の配列に置く。
//   遅延は 2 つ持つ。部分木の遅延 (b, c, rev) は子へ配る分、塊の遅延 (cb, cc, crev) は自分の配列へ当てる分で、配列を書き換える
//   ときにだけ当てる (materialize)。節の sum と csum (自分の配列の和) は、どちらの遅延も当てたあとの値を持つ。
//   区間の和と 1 次関数の作用は形を変えずに根から下り、端の塊だけ配列を書き換える。挿入と削除は、その位置の塊の配列を
//   ずらす (満杯なら塊を割る)。反転は split を 2 回して真ん中の木に印を付け、merge で戻す (端の塊を割るので節が増える)。
//   節が増えたら、値を並べ直して B / 2 個ずつの塊で木を作り直す。
struct Solver {
  using Z = ZMod<998244353>;
  static constexpr int B = 128;
  struct Node {
    int l, r, cnt, len;
    unsigned pri;
    Z sum, csum, b, c, cb, cc;
    bool rev, lz, crev, clz;
  };
  vector<Node> t;
  vector<Z> dat;  // 節 x の配列は dat[x * B, x * B + len)
  int root = 0, limit = 0;
  unsigned rs = 2463534242u;
  unsigned rnd() { return rs ^= rs << 13, rs ^= rs >> 17, rs ^= rs << 5; }
  int make(unsigned pri) {
    t.push_back({0, 0, 0, 0, pri, Z(), Z(), Z(1), Z(), Z(1), Z(), false, false, false, false});
    if (dat.size() < t.size() * B) dat.resize(t.size() * B * 2);
    return (int)t.size() - 1;
  }
  Z *arr(int x) { return dat.data() + size_t(x) * B; }
  void aff(int x, Z b, Z c) {
    Node &n = t[x];
    n.sum = b * n.sum + c * Z(n.cnt), n.csum = b * n.csum + c * Z(n.len);
    n.b = b * n.b, n.c = b * n.c + c, n.lz = true;
    n.cb = b * n.cb, n.cc = b * n.cc + c, n.clz = true;
  }
  void tog(int x) { swap(t[x].l, t[x].r), t[x].rev ^= 1, t[x].crev ^= 1; }
  void push(int x) {
    Node &n = t[x];
    if (n.rev) {
      if (n.l) tog(n.l);
      if (n.r) tog(n.r);
      n.rev = false;
    }
    if (n.lz) {
      if (n.l) aff(n.l, n.b, n.c);
      if (n.r) aff(n.r, n.b, n.c);
      n.b = Z(1), n.c = Z(), n.lz = false;
    }
  }
  void materialize(int x) {
    Node &n = t[x];
    Z *a = arr(x);
    if (n.clz) {
      for (int i = 0; i < n.len; ++i) a[i] = n.cb * a[i] + n.cc;
      n.cb = Z(1), n.cc = Z(), n.clz = false;
    }
    if (n.crev) std::reverse(a, a + n.len), n.crev = false;
  }
  void recalc(int x) {
    Z s;
    const Z *a = arr(x);
    for (int i = 0; i < t[x].len; ++i) s += a[i];
    t[x].csum = s;
  }
  void pull(int x) {
    Node &n = t[x];
    n.cnt = t[n.l].cnt + n.len + t[n.r].cnt, n.sum = t[n.l].sum + n.csum + t[n.r].sum;
  }
  // 先頭の k 個を a に、残りを b に分ける。k が塊の中に落ちたら、塊を 2 つの節に割る。
  void split(int x, int k, int &a, int &b) {
    if (!x) return void(a = b = 0);
    push(x);
    const int ls = t[t[x].l].cnt, len = t[x].len;
    if (k <= ls) {
      int p, q;
      split(t[x].l, k, p, q);
      t[x].l = q, pull(x), a = p, b = x;
    } else if (k >= ls + len) {
      int p, q;
      split(t[x].r, k - ls - len, p, q);
      t[x].r = p, pull(x), a = x, b = q;
    } else {
      const int o = k - ls;
      materialize(x);
      const int y = make(t[x].pri);  // y は x の右の部分木を引き継ぐので、優先度を x と同じにして山の条件を保つ
      std::copy(arr(x) + o, arr(x) + len, arr(y));
      t[y].len = len - o, t[x].len = o;
      recalc(x), recalc(y);
      t[y].r = t[x].r, t[x].r = 0;
      pull(x), pull(y);
      a = x, b = y;
    }
  }
  int merge(int a, int b) {
    if (!a || !b) return a | b;
    if (t[a].pri > t[b].pri) {
      push(a);
      const int r = merge(t[a].r, b);
      t[a].r = r, pull(a);
      return a;
    }
    push(b);
    const int l = merge(a, t[b].l);
    t[b].l = l, pull(b);
    return b;
  }
  Z prod(int x, int l, int r) {
    if (!x || l >= r) return Z();
    if (l == 0 && r == t[x].cnt) return t[x].sum;
    push(x);
    const int ls = t[t[x].l].cnt, len = t[x].len;
    Z s;
    if (l < ls) s += prod(t[x].l, l, min(r, ls));
    const int cl = max(l - ls, 0), cr = min(r - ls, len);
    if (cl < cr) {
      if (cl == 0 && cr == len) s += t[x].csum;
      else {
        materialize(x);
        const Z *a = arr(x);
        for (int i = cl; i < cr; ++i) s += a[i];
      }
    }
    if (r > ls + len) s += prod(t[x].r, max(l - ls - len, 0), r - ls - len);
    return s;
  }
  void apply(int x, int l, int r, Z b, Z c) {
    if (!x || l >= r) return;
    if (l == 0 && r == t[x].cnt) return aff(x, b, c);
    push(x);
    const int ls = t[t[x].l].cnt, len = t[x].len;
    if (l < ls) apply(t[x].l, l, min(r, ls), b, c);
    const int cl = max(l - ls, 0), cr = min(r - ls, len);
    if (cl < cr) {
      materialize(x);
      Z *a = arr(x);
      for (int i = cl; i < cr; ++i) a[i] = b * a[i] + c;
      recalc(x);
    }
    if (r > ls + len) apply(t[x].r, max(l - ls - len, 0), r - ls - len, b, c);
    pull(x);
  }
  // 値を並べ直して、B / 2 個ずつの塊で釣り合った木を作り直す。
  void collect(int x, vector<Z> &out) {
    if (!x) return;
    push(x), materialize(x);
    collect(t[x].l, out);
    out.insert(out.end(), arr(x), arr(x) + t[x].len);
    collect(t[x].r, out);
  }
  int build(const vector<Z> &v, int lo, int hi, int depth) {  // 塊の番号の [lo, hi)
    if (lo >= hi) return 0;
    const int m = (lo + hi) / 2;
    const int x = make(~0u - (unsigned(depth) << 24) - (rnd() & 0xffffffu));  // 浅いほど優先度が高い
    const int s = m * (B / 2), e = min<int>(s + B / 2, (int)v.size());
    std::copy(v.begin() + s, v.begin() + e, arr(x));
    t[x].len = e - s, recalc(x);
    const int l = build(v, lo, m, depth + 1), r = build(v, m + 1, hi, depth + 1);
    t[x].l = l, t[x].r = r, pull(x);
    return x;
  }
  void rebuild(const vector<Z> &v) {
    t.resize(1), root = 0;
    const int chunks = ((int)v.size() + B / 2 - 1) / (B / 2);
    root = build(v, 0, chunks, 0);
    limit = 2 * (int)t.size() + 1024;
  }
  void maybe_rebuild() {
    if ((int)t.size() <= limit) return;
    vector<Z> v;
    v.reserve(t[root].cnt);
    collect(root, v);
    rebuild(v);
  }
  explicit Solver(const vector<i64> &a) {
    t.reserve(1 << 16);
    t.push_back({0, 0, 0, 0, 0, Z(), Z(), Z(1), Z(), Z(1), Z(), false, false, false, false});  // 0 は空の木
    dat.resize(size_t(1 << 12) * B);
    vector<Z> v(a.begin(), a.end());
    rebuild(v);
  }
  void insert(int i, i64 v) {
    if (!root) {
      root = make(rnd());
      arr(root)[0] = Z(v), t[root].len = 1, recalc(root), pull(root);
      return;
    }
    // i を含む塊 (塊の後ろの端も含む) まで下りる。
    static vector<int> path;
    path.clear();
    int x = root, o = i;
    for (;;) {
      push(x), path.push_back(x);
      const int ls = t[t[x].l].cnt;
      if (o < ls) x = t[x].l;
      else if (o <= ls + t[x].len) {
        o -= ls;
        break;
      } else o -= ls + t[x].len, x = t[x].r;
    }
    if (t[x].len < B) {
      materialize(x);
      Z *a = arr(x);
      std::copy_backward(a + o, a + t[x].len, a + t[x].len + 1);
      a[o] = Z(v), ++t[x].len, t[x].csum += Z(v);
      for (int k = (int)path.size(); k--;) pull(path[k]);
      return;
    }
    // 塊が満杯なら、split と merge で 1 つの値の節を挟む。
    int a, b;
    split(root, i, a, b);
    const int y = make(rnd());
    arr(y)[0] = Z(v), t[y].len = 1, recalc(y), pull(y);
    root = merge(merge(a, y), b);
    maybe_rebuild();
  }
  void erase(int i) {
    static vector<int> path;
    path.clear();
    int x = root, o = i;
    for (;;) {
      push(x), path.push_back(x);
      const int ls = t[t[x].l].cnt;
      if (o < ls) x = t[x].l;
      else if (o < ls + t[x].len) {
        o -= ls;
        break;
      } else o -= ls + t[x].len, x = t[x].r;
    }
    materialize(x);
    Z *a = arr(x);
    t[x].csum -= a[o];
    std::copy(a + o + 1, a + t[x].len, a + o);
    --t[x].len;
    for (int k = (int)path.size(); k--;) pull(path[k]);
  }
  void reverse(int l, int r) {
    if (r - l < 2) return;
    int a, b, c;
    split(root, l, a, b);
    split(b, r - l, b, c);
    tog(b);
    root = merge(merge(a, b), c);
    maybe_rebuild();
  }
  void affine(int l, int r, i64 b, i64 c) { apply(root, l, r, Z(b), Z(c)); }
  i64 sum(int l, int r) { return prod(root, l, r).val(); }
};
