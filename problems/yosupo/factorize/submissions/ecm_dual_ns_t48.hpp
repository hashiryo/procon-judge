#pragma once
// 2 本同時の ECM で、正規化した 1 本の ladder と、組を飛ばす stage 2 の両方を使うもの (B1 = 150、m <= 16)。中身は _ecm2.hpp。
#include "_ecm2.hpp"
inline vector<vector<u64>> run(const vector<u64>& qs) {
 vector<vector<u64>> ans;
 ans.reserve(qs.size());
 for(u64 x: qs) ans.push_back(ecm_fact::factorize2<true, true>(x));
 return ans;
}
