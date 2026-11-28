#include <algorithm>
#include <iostream>
#include <utility>
#include <vector>
#include "neo/base/core/algebra/ZMod.hpp"
#include "neo/base/core/nt/factorial.hpp"

int main() {
 std::cin.tie(nullptr);
 std::ios::sync_with_stdio(false);
 using Z= ZMod<0>;
 int T;
 unsigned m;
 std::cin >> T >> m;
 Z::set_mod(m);
 std::vector<std::pair<int, int>> qs(T);
 int mx= 0;
 for(auto& [n, k]: qs) std::cin >> n >> k, mx= std::max(mx, n);
 Factorial<Z> f(mx);
 for(auto [n, k]: qs) std::cout << f.nCr(n, k).val() << '\n';
}
