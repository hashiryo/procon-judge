#include <iostream>
#include "neo/algo/compute/nt/tetration.hpp"

signed main() {
 std::cin.tie(nullptr);
 std::ios::sync_with_stdio(false);
 int T;
 std::cin >> T;
 while(T--) {
  unsigned long long A, B, M;
  std::cin >> A >> B >> M;
  std::cout << mod_tetration(A, B, M) << '\n';
 }
}
