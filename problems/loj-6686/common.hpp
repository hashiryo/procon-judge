#pragma once
// ライブラリを使う提出が共有するもの。立方根 l を二分探索で出し、floor(cbrt(i)) = l
// になる末尾の区間 [l^3, n] を l の約数ごとに数えるところはどちらの実装でも同じ
// なので、ここに置く。残りの [1, l^3) の和が比べたいところ。
#include <cstdint>
#include "pj.hpp"
#include "mylib/algebra/ModInt.hpp"
#include "mylib/number_theory/ArrayOnDivisors.hpp"

using Mint = ModInt<998244353>;
using u128 = unsigned __int128;

struct Split {
  uint64_t l;  // floor(cbrt(n))
  uint64_t r;  // l - 1。ここまでの立方根で [1, l^3) が尽きる
  Mint tail;   // [l^3, n] の gcd(l, i) の和
};

inline Split split(u128 n) {
  uint64_t l = 0, h = 10000000010ULL;
  while (h - l > 1) {
    uint64_t x = (h + l) / 2;
    u128 t = u128(x) * x * x;
    if (t <= n) l = x;
    else h = x;
  }
  const u128 m = u128(l) * l * l;
  Split s{l, l - 1, Mint()};
  // gcd(l, i) = sum_{d | gcd(l, i)} phi(d) なので、l の約数 d ごとに d の倍数を数える。
  ArrayOnDivisors<uint64_t, uint64_t> totient(l);
  totient.set_totient();
  for (auto [d, phi] : totient) s.tail += Mint(n / d - (m - 1) / d) * phi;
  return s;
}
