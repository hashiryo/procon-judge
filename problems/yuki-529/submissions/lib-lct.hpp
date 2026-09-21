#pragma once
#include "common.hpp"
#include "mylib/data_structure/LinkCutTree.hpp"

// Link-Cut 木。道の最大を expose だけで出せるので、HLD のような区間への分解が
// 要らない。最大は可換なので、反転したときの値を別に持たなくてよい。
struct Solver {
  BridgeTree tree;
  Prey prey;
  LinkCutTree<MaxPrey> lct;

  Solver(int n, const vector<array<int, 2>> &edges)
      : tree(contract(n, edges)), prey(tree.n), lct(tree.n) {
    for (int c = 0; c < tree.n; ++c) lct.set(c, {-1, c});
    for (auto &e : tree.edges) lct.link(e[0], e[1]);
  }

  void add(int u, i64 w) {
    int c = tree.id[u];
    prey.add(c, w);
    lct.set(c, {prey.top(c), c});
  }

  i64 take(int s, int t) {
    int u = tree.id[s], v = tree.id[t];
    auto [w, c] = lct.prod(u, v);
    if (w == -1) return -1;
    prey.pop(c);
    lct.set(c, {prey.top(c), c});
    return w;
  }
};
