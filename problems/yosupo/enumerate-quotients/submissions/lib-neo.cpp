#include <iostream>
#include "neo/base/core/nt/quotient.hpp"

signed main() {
 std::cin.tie(nullptr);
 std::ios::sync_with_stdio(false);
 unsigned long long N;
 std::cin >> N;
 auto ans= enumerate_quotients(N);
 const int k= ans.size();
 std::cout << k << '\n';
 for(int i= 0; i < k; ++i) std::cout << std::get<0>(ans[i]) << " \n"[i == k - 1];
}
