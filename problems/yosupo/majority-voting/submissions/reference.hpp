#pragma once
// セグメント木に Boyer-Moore の (候補, 差) を持たせ、区間の候補を O(log N) で出す。
// 候補が本当に過半数かを確かめるために、値ごとに「その値が入りうる位置」を集めて
// 座標圧縮し、その上に Fenwick 木を持つ。区間内の個数は O(log N) で数えられる。
// 位置の候補は初期の列とタイプ 0 の x から先に全部集める (構築は計測区間の外)。
#include <algorithm>
#include <array>
#include <utility>
#include <vector>

struct Solver {
  // セグメント木の (圧縮した値, 差)。空区間は {-1, 0}。
  using Node = std::pair<int, int>;

  const std::vector<std::array<int, 3>> &queries;
  int n;
  int sz;                              // セグメント木の葉の数 (2 の冪)
  std::vector<int> a;                  // 現在の値 (圧縮後)
  std::vector<int> vals;               // 圧縮の対応 (元の値の昇順)
  std::vector<std::vector<int>> at;    // at[v]: 値 v が入りうる位置 (昇順)
  std::vector<std::vector<int>> fen;   // fen[v]: at[v] の各位置が今 v かを数える Fenwick
  std::vector<Node> seg;
  std::vector<int> ans;

  // Boyer-Moore の併合。同じ値なら差を足し、違えば大きい方から小さい方を引く。
  static Node op(Node x, Node y) {
    if (x.first < 0) return y;
    if (y.first < 0) return x;
    if (x.first == y.first) return {x.first, x.second + y.second};
    if (x.second >= y.second) return {x.first, x.second - y.second};
    return {y.first, y.second - x.second};
  }

  static void fen_add(std::vector<int> &f, int i, int d) {
    for (++i; i < (int)f.size(); i += i & -i) f[i] += d;
  }

  // [0, i) の和。
  static int fen_sum(const std::vector<int> &f, int i) {
    int s = 0;
    for (; i > 0; i -= i & -i) s += f[i];
    return s;
  }

  int comp(int x) const {
    return (int)(std::lower_bound(vals.begin(), vals.end(), x) - vals.begin());
  }

  Solver(std::vector<int> a_, const std::vector<std::array<int, 3>> &q)
      : queries(q), n((int)a_.size()) {
    // 初期の値と更新で入る値の両方を圧縮する。
    vals.reserve(a_.size() + queries.size());
    for (int x : a_) vals.push_back(x);
    for (const auto &e : queries)
      if (e[0] == 0) vals.push_back(e[2]);
    std::sort(vals.begin(), vals.end());
    vals.erase(std::unique(vals.begin(), vals.end()), vals.end());
    int m = (int)vals.size();

    // 値ごとに、その値が入りうる位置を集める。更新でその位置に来ることも含める。
    a.resize(n);
    at.assign(m, {});
    for (int i = 0; i < n; ++i) {
      a[i] = comp(a_[i]);
      at[a[i]].push_back(i);
    }
    for (const auto &e : queries)
      if (e[0] == 0) at[comp(e[2])].push_back(e[1]);
    fen.resize(m);
    for (int v = 0; v < m; ++v) {
      auto &p = at[v];
      std::sort(p.begin(), p.end());
      p.erase(std::unique(p.begin(), p.end()), p.end());
      fen[v].assign(p.size() + 1, 0);
    }

    // 今の値で Fenwick を初期化する。
    for (int i = 0; i < n; ++i) {
      int j = (int)(std::lower_bound(at[a[i]].begin(), at[a[i]].end(), i) -
                    at[a[i]].begin());
      fen_add(fen[a[i]], j, 1);
    }

    // セグメント木を組む。
    sz = 1;
    while (sz < n) sz <<= 1;
    seg.assign(2 * sz, {-1, 0});
    for (int i = 0; i < n; ++i) seg[sz + i] = {a[i], 1};
    for (int k = sz - 1; k >= 1; --k) seg[k] = op(seg[2 * k], seg[2 * k + 1]);

    ans.reserve(queries.size());
  }

  Node fold(int l, int r) const {
    Node sl{-1, 0}, sr{-1, 0};
    l += sz;
    r += sz;
    while (l < r) {
      if (l & 1) sl = op(sl, seg[l++]);
      if (r & 1) sr = op(seg[--r], sr);
      l >>= 1;
      r >>= 1;
    }
    return op(sl, sr);
  }

  void run() {
    for (const auto &e : queries) {
      if (e[0] == 0) {
        int p = e[1];
        int x = comp(e[2]);
        int old = a[p];
        if (old == x) continue;
        int k = p + sz;
        seg[k] = {x, 1};
        for (k >>= 1; k; k >>= 1) seg[k] = op(seg[2 * k], seg[2 * k + 1]);
        int j = (int)(std::lower_bound(at[old].begin(), at[old].end(), p) -
                      at[old].begin());
        fen_add(fen[old], j, -1);
        j = (int)(std::lower_bound(at[x].begin(), at[x].end(), p) -
                  at[x].begin());
        fen_add(fen[x], j, 1);
        a[p] = x;
      } else {
        int l = e[1], r = e[2];
        int v = fold(l, r).first;
        if (v < 0) {  // 空区間は来ないが、念のため。
          ans.push_back(-1);
          continue;
        }
        int li = (int)(std::lower_bound(at[v].begin(), at[v].end(), l) -
                       at[v].begin());
        int ri = (int)(std::lower_bound(at[v].begin(), at[v].end(), r) -
                       at[v].begin());
        int c = fen_sum(fen[v], ri) - fen_sum(fen[v], li);
        ans.push_back(2 * c > r - l ? vals[v] : -1);
      }
    }
  }

  const std::vector<int> &answers() const { return ans; }
};
