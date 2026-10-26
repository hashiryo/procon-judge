// NeoLibrary の XorBasis (neo/linalg/XorBasis.hpp) で書いたもの。作れる値の数は、張る空間の元の数 2^(基底の大きさ)。
#include <cstdio>
#include "neo/linalg/XorBasis.hpp"
int main() {
 int n;
 if(scanf("%d", &n) != 1) return 1;
 XorBasis<unsigned long long> b;
 for(int i= 0; i < n; ++i) {
  unsigned long long a;
  if(scanf("%llu", &a) != 1) return 1;
  b.insert(a);
 }
 printf("%llu\n", (unsigned long long)b.count());
}
