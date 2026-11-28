// 二項係数 (合成数 mod) のうち、n / pp と n / q を runtime の 64 bit 割り算のままにした版。
// neo の binomial.hpp が p = 2 のときシフトに替えたのと書き比べる (核は problems/_shared/nt/binomial_div.hpp)。
#include <iostream>
#include "_shared/nt/binomial_div.hpp"

int main() {
 std::cin.tie(nullptr);
 std::ios::sync_with_stdio(false);
 int T, m;
 std::cin >> T >> m;
 const BinomialCoefficient binom(m);
 while(T--) {
  long long n, k;
  std::cin >> n >> k;
  std::cout << binom.nCr(n, k) << '\n';
 }
}
