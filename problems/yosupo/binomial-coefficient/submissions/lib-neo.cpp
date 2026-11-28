#include <iostream>
#include "neo/algo/compute/nt/binomial.hpp"

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
