// 区間 1 つあたりの割り算を 1 回にした版 (核は problems/_shared/nt/quotient_alt.hpp)。
#include <iostream>
#include "_shared/nt/quotient_alt.hpp"

signed main() {
 std::cin.tie(nullptr);
 std::ios::sync_with_stdio(false);
 unsigned long long N;
 std::cin >> N;
 auto ans= enumerate_quotients_alt(N);
 const int k= ans.size();
 std::cout << k << '\n';
 for(int i= 0; i < k; ++i) std::cout << std::get<0>(ans[i]) << " \n"[i == k - 1];
}
