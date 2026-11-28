// 今の Library の OrderFp (constexpr、位数を素因数の積の二分木でまとめて求める) で、neo の lib-neo.cpp と同じ確認をする。
// 順位表に Library Checker の位数の問題が無いので、この自作問題で neo と書き比べる。
#include <cassert>
#include <random>
#include <vector>
#include "mylib/number_theory/OrderFp.hpp"
#include "mylib/number_theory/binary_gcd.hpp"
#include "mylib/number_theory/is_prime.hpp"

using u64= unsigned long long;
using u128= unsigned __int128;

static u64 pow_mod(u64 a, u64 e, u64 m) {
 u64 r= 1;
 for(a%= m; e; e>>= 1, a= (u64)((u128)a * a % m))
  if(e & 1) r= (u64)((u128)r * a % m);
 return r;
}

signed main() {
 {  // 998244353 の既知の値
  OrderFp ord(998244353);
  assert(ord.primitive_root() == 3);
  assert(ord(0) == 0);
  assert(ord(1) == 1);
  assert(ord(2) == 499122176);
  assert(ord(3) == 998244352);
  assert(ord(4) == 249561088);
 }

 // 小さい素数は、掛け続けて 1 に戻るまでの回数と比べる。
 for(u64 p= 2; p < 500; ++p) {
  if(!is_prime(p)) continue;
  OrderFp ord(p);
  for(u64 x= 1; x < p; ++x) {
   u64 o= 1, y= x;
   while(y != 1) y= (u64)((u128)y * x % p), ++o;
   assert(ord(x) == o);
  }
 }

 // 64 ビットの素数では、位数が p-1 を割ること、x^位数 = 1、g^k の位数が (p-1)/gcd(p-1,k) になることを確かめる。
 std::mt19937_64 rng(123456789);
 std::vector<u64> ps= {2305843009213693951ull, 1000000007ull, 2147483647ull};
 for(int i= 0; i < 8;) {
  u64 p= rng() | 1;
  if(p < 3 || !is_prime(p)) continue;
  ps.push_back(p), ++i;
 }
 for(u64 p: ps) {
  OrderFp ord(p);
  const u64 g= ord.primitive_root();
  assert(ord(g) == p - 1);
  for(int t= 0; t < 20; ++t) {
   const u64 x= rng() % p;
   const u64 o= ord(x);
   assert(o == 0 || (p - 1) % o == 0);
   assert(o == 0 || pow_mod(x, o, p) == 1);
   const u64 k= 1 + rng() % (p - 1);
   assert(ord(pow_mod(g, k, p)) == (p - 1) / binary_gcd(p - 1, k));
  }
 }
 return 0;
}
