#pragma once
// 今の Library の ModInt<998244353> をそのまま使う。整数は MInt(__int128_t) で受けるので、i64 からの変換で __modti3 を呼び、
// そのあと Montgomery の表現に直す (MP_Mo32 の set)。mylib/algebra/ModInt.hpp が変わると測り直される。
#include "_shared/modulo-test/_common.hpp"
#include "mylib/algebra/ModInt.hpp"
struct MP {
 using M= ModInt<998244353>;
 constexpr MP(u32) {}
 constexpr inline M set(u32 n) const { return M(n); }
 constexpr inline u32 get(M v) const { return v.val(); }
 constexpr inline M plus(M l, M r) const { return l + r; }
 constexpr inline M from_i64(long long a) const { return M(a); }
};
