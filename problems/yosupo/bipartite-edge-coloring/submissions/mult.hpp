#pragma once
// 多重辺を辺と重複度にまとめ、偶数の次数は重複度の半分ずつと Euler 閉路で分け、奇数の次数は完全マッチング (NeoLibrary の
// BipartiteMatching) を 1 つ取る。辺の種類が 64 以下になったら、完全マッチングの上の重複度の最小だけ色をまとめて塗る
// (核は _shared/flow/bec_mult.hpp)。
#include "_shared/flow/bec_mult.hpp"
struct Solver {
 int l, r;
 const vector<array<int, 2>>& e;
 vector<int> c;
 Solver(int l, int r, const vector<array<int, 2>>& edges): l(l), r(r), e(edges) {}
 void run() { c= bec_mult::edge_coloring<64>(l, r, e); }
 const vector<int>& colors() const { return c; }
};
