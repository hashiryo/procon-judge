#pragma once
// Library の enumerate_primes (奇数だけの篩) をハーネスの形に合わせる。
#include "../common.hpp"
#include "mylib/number_theory/enumerate_primes.hpp"
struct Solver {
 static std::pair<u32, std::vector<u32>> run(u32 N, u32 A, u32 B) {
  auto ps= enumerate_primes(N);
  u32 pi= ps.size();
  std::vector<u32> sel;
  for(u64 i= B; i < pi; i+= A) sel.push_back(ps[i]);
  return {pi, sel};
 }
};
