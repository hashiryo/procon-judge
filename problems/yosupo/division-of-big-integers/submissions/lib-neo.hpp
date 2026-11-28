#pragma once
// NeoLibrary の BigInt (neo/base/core/nt/BigInt.hpp) で、多倍長整数の割り算を解く。
// 桁は 10^9 進の u32、積は 3 素数の NTT、割り算は小さい側を Knuth の D、大きい側を Newton 法。
#include <string>
#include "neo/base/core/nt/BigInt.hpp"

struct Solver {
  BigInt a, b, q, r;
  Solver(const std::string &sa, const std::string &sb) : a(sa), b(sb) {}
  void run() { divmod(a, b, q, r); }
  std::string quotient() const { return q.to_string(); }
  std::string remainder() const { return r.to_string(); }
};
