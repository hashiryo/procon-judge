// neo の Factorial<ZMod<998244353>> (法が型の定数) で解く。
#pragma once
#include <vector>
#include "neo/base/core/algebra/ZMod.hpp"
#include "neo/base/core/nt/factorial.hpp"

struct Solver {
 std::vector<unsigned> run(unsigned N, const std::vector<unsigned>& ns, const std::vector<unsigned>& rs) {
  using Z= ZMod<998244353>;
  (void)N;
  Factorial<Z> f;  // 問いの n に合わせて延びる (ensure)
  std::vector<unsigned> out;
  out.reserve(ns.size());
  for (std::size_t i= 0; i < ns.size(); ++i) out.push_back(f.nCr((int)ns[i], (int)rs[i]).val());
  return out;
 }
};
