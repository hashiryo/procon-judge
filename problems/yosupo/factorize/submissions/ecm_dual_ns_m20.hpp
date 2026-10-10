#pragma once
// ecm_dual_ns_t48 の m の上限を 20 にしたもの (B2 ≈ 4300)。中身は _ecm2.hpp。
#include "_ecm2.hpp"
inline vector<vector<u64>> run(const vector<u64>& qs) {
 vector<vector<u64>> ans;
 ans.reserve(qs.size());
 for(u64 x: qs) ans.push_back(ecm_fact::factorize2<true, true, 150, 20>(x));
 return ans;
}
