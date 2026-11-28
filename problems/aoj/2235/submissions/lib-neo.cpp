#include <iostream>
#include "neo/base/ds/tree/OnlineDynamicConnectivity.hpp"
using namespace std;
// lib.cpp と同じ形で、動的連結性だけ NeoLibrary のものにした (値を持たない M = void)。
// 段の ETT の置き場は型ごとに 1 つなので、置き場の大きさを NODE_SIZE で渡す。
// この問題は n と k が 38702 までなので、1e6 節を取る。
int main() {
 std::cin.tie(nullptr);
 std::ios::sync_with_stdio(false);
 int n, k;
 cin >> n >> k;
 OnlineDynamicConnectivity<void, 1'000'000> dicon(n);
 while (k--) {
  int op, u, v;
  cin >> op >> u >> v;
  if (op == 1) dicon.link(u, v);
  if (op == 2) dicon.cut(u, v);
  if (op == 3) cout << (dicon.connected(u, v) ? "YES" : "NO") << '\n';
 }
}
