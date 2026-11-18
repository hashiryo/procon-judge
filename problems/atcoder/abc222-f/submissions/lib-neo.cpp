// lib.cpp を NeoLibrary の Rerooting (木は辺の列で渡す) で書いたもの。辺の向こうの頂点は両端の xor で引く。
#include <array>
#include <iostream>
#include <vector>
#include "neo/graph/Rerooting.hpp"
using namespace std;
signed main() {
 cin.tie(0);
 ios::sync_with_stdio(0);
 int N;
 cin >> N;
 vector<array<int, 2>> es(N - 1);
 vector<int> C(N - 1);
 for(int i= 0; i < N - 1; ++i) cin >> es[i][0] >> es[i][1] >> C[i], --es[i][0], --es[i][1];
 vector<long long> D(N);
 for(int i= 0; i < N; ++i) cin >> D[i];
 auto put_edge= [&](int v, int e, long long d) { return max(d, D[es[e][0] ^ es[e][1] ^ v]) + C[e]; };
 auto op= [&](long long l, long long r) { return max(l, r); };
 auto put_vertex= [&](int, long long d) { return d; };
 for(long long x: Rerooting<long long>(N, es, put_edge, op, 0ll, put_vertex)) cout << x << '\n';
 return 0;
}
