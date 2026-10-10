#pragma once
// NeoLibrary の is_prime (表を持たない BPSW、bpsw_es_top3.hpp と同じ判定) を呼ぶ。
// lib.hpp は今の Library (mylib) の is_prime。
#include "../common.hpp"
#include "neo/number_theory/is_prime.hpp"

inline vector<bool> run(const vector<u64>& qs) {
  vector<bool> ans(qs.size());
  for (size_t i = 0; i < qs.size(); ++i) ans[i] = is_prime(qs[i]);
  return ans;
}
