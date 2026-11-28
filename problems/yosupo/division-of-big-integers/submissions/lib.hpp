#pragma once
// 今の Library の BigInt (mylib/fft/BigInt.hpp) で、多倍長整数の割り算を解く。
// 桁は 10^15 進の u64 で、積は 4 素数の NTT、割り算は Newton 法。
#include <string>
#include "mylib/fft/BigInt.hpp"

struct Solver {
  BigInt a, b, q, r;
  Solver(const std::string &sa, const std::string &sb) : a(sa), b(sb) {}
  void run() {
    q = a / b;
    r = a - q * b;
  }
  std::string quotient() const { return q.to_str(); }
  std::string remainder() const { return r.to_str(); }
};
