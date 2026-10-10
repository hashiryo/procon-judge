#pragma once
// ecm_dual_ns_t48 の B1 を 125 にしたもの (m <= 16)。中身は _ecm2.hpp。
#include "_ecm2.hpp"
inline vector<vector<u64>> run(const vector<u64>& qs) {
 vector<vector<u64>> ans;
 ans.reserve(qs.size());
 for(u64 x: qs) ans.push_back(ecm_fact::factorize2<true, true, 125, 16>(x));
 return ans;
}
