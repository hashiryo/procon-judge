#pragma once
// Library の binary_gcd (分岐の無い Stein のアルゴリズム) をそのまま使う。
// 旧 judge の binary_lib.hpp はこの写しだった。mylib/number_theory/binary_gcd.hpp が変わると測り直される。
#include "../../_shared/modulo-test/_common.hpp"
#include "mylib/number_theory/binary_gcd.hpp"
struct G {
 static inline u64 gcd(u64 a, u64 b) { return binary_gcd(a, b); }
};
