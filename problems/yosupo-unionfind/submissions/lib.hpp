#pragma once
// Library の UnionFind をハーネスの形 (unite / same) に合わせる。
#include "../common.hpp"
#include "mylib/data_structure/UnionFind.hpp"
struct Solver {
 UnionFind uf;
 explicit Solver(int n): uf(n) {}
 void unite(int a, int b) { uf.unite(a, b); }
 bool same(int a, int b) { return uf.connected(a, b); }
};
