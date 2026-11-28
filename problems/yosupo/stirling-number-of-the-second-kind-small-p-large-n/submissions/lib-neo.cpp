#include <iostream>
#include "neo/algo/compute/nt/stirling.hpp"

signed main() {
 std::cin.tie(nullptr);
 std::ios::sync_with_stdio(false);
 int T;
 unsigned p;
 std::cin >> T >> p;
 StirlingNumber sn(p, false, true);
 while(T--) {
  unsigned long long n, k;
  std::cin >> n >> k;
  std::cout << sn.S2(n, k) << '\n';
 }
}
