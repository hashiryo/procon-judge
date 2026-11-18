// 双対の verify。lib.cpp の木を NeoLibrary の HeavyLightDecomposition に、剰余を ZMod にしたもの。(位置, 深さ) の点への 1 次関数の
// 作用は、今の Library の kd 木で持つ (NeoLibrary に kd 木はまだ無い)。
#include <iostream>
#include <array>
#include <tuple>
#include <vector>
#include "mylib/data_structure/KDTree.hpp"
#include "neo/algebra/ZMod.hpp"
#include "neo/graph/HeavyLightDecomposition.hpp"
using namespace std;
using Mint= ZMod<998244353>;
struct RaffineQ {
 using T= Mint;
 using E= array<Mint, 2>;
 static void mp(T& v, const E& x) { v= x[0] * v + x[1]; }
 static void cp(E& p, const E& n) { p[0]*= n[0], p[1]*= n[0], p[1]+= n[1]; }
};
signed main() {
 cin.tie(0);
 ios::sync_with_stdio(0);
 int N, Q;
 cin >> N >> Q;
 vector<array<int, 2>> es(N - 1);
 for(auto& e: es) cin >> e[0] >> e[1], --e[0], --e[1];
 HeavyLightDecomposition tree(N, es, 0);
 vector<tuple<int, int, Mint>> xy;
 for(int v= 0; v < N; ++v) {
  long long X;
  cin >> X;
  xy.emplace_back(tree.to_seq(v), tree.depth(v), Mint(X));
 }
 KDTree<int, 2, RaffineQ> kdt(xy);
 while(Q--) {
  int t;
  cin >> t;
  if(t == 1) {
   int V;
   cin >> V, --V;
   cout << kdt.get(tree.to_seq(V), tree.depth(V)).val() << "\n";
  } else if(t == 2) {
   int V, K, C, D;
   cin >> V >> K >> C >> D, --V;
   for(int i= 0; i <= K; ++i) {
    int p= tree.parent(V);
    auto [l, r]= tree.subtree(V);
    int d= tree.depth(V);
    if(p == -1) {
     kdt.apply_cuboid(l, r - 1, d, d + K - i, {Mint(C), Mint(D)});
     break;
    }
    kdt.apply_cuboid(l, r - 1, d + K - i - 1, d + K - i, {Mint(C), Mint(D)});
    V= p;
   }
  } else if(t == 3) {
   int V, C, D;
   cin >> V >> C >> D, --V;
   auto [l, r]= tree.subtree(V);
   kdt.apply_cuboid(l, r - 1, 0, N, {Mint(C), Mint(D)});
  } else {
   int U, V, C, D;
   cin >> U >> V >> C >> D, --U, --V;
   tree.path(U, V, [&](int a, int b) { a < b ? kdt.apply_cuboid(a, b, 0, N, {Mint(C), Mint(D)}) : kdt.apply_cuboid(b, a, 0, N, {Mint(C), Mint(D)}); });
  }
 }
 return 0;
}
