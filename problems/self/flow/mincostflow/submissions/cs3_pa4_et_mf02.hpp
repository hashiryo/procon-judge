#pragma once
// 実験。cs3_pa4_et と同じ中身を、NeoLibrary 02f478d の MaxFlow (_MaxFlow_02f478d.hpp) で組む。37f7c53 を測った回 (EPYC 9V45) で、
// cs3_pa4_et の x64-gcc の seed 0〜2 が前の回の 1.3〜1.6 倍になった。最初の実行可能性の判定の最大流は手元で 10〜40 ms で、
// b_flow の機械語が MaxFlow の版で変わっていた。同じ束の中で、MaxFlow の版だけを変えて差が出るかを見る。
#include "_mcf_cs3_mf02.hpp"
struct Solver {
 int n;
 const vector<i64>& b;
 const vector<array<i64, 5>>& e;
 mcf_cs3::Result r;
 Solver(int n, const vector<i64>& b, const vector<array<i64, 5>>& edges): n(n), b(b), e(edges) {}
 void run() { r= mcf_cs3::b_flow<16, 4, 2, 1, 1, 100>(n, b, e); }
 bool feasible() const { return r.ok; }
 __int128 cost() const { return r.cost; }
 const vector<i64>& potential() const { return r.pot; }
 const vector<i64>& flow() const { return r.flow; }
};
