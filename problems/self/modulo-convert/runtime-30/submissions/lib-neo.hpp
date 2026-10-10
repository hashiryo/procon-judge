#pragma once
// NeoLibrary の ZMod<0> (法を実行時に決める型) をそのまま使い、使う側が書く形のまま演算子で計算する。
// ハーネスが呼ぶ口 (set、get、plus、mul、pow、fix、mul_fix、from_i64、dot) をまとめて持ち、問題ごとに使うものだけが呼ばれる。
// neo/algebra/ZMod.hpp が変わると測り直される。
#include "_shared/modulo-test/_common.hpp"
#include "neo/algebra/ZMod.hpp"
struct MP {
 using Z= ZMod<0>;
 MP(u32 m) { Z::set_mod(m); }
 Z set(u32 n) const { return Z(n); }
 u32 get(Z v) const { return v.val(); }
 Z plus(Z l, Z r) const { return l + r; }
 Z mul(Z l, Z r) const { return l * r; }
 Z pow(Z a, u64 e) const { return a.pow(e); }
 typename Z::Fixed fix(Z w) const { return w.fixed(); }
 Z mul_fix(Z a, typename Z::Fixed w) const { return a * w; }
 Z from_i64(long long a) const { return Z(a); }
 Z dot(const Z* a, const Z* b, size_t n) const {
  Z s;
  for(size_t k= 0; k < n; ++k) s+= a[k] * b[k];
  return s;
 }
};
