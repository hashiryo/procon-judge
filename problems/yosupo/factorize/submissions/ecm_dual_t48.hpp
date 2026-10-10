#pragma once
// 2 本の曲線を同時に回す ECM で割る (しきい値 2^48)。中身は _ecm.hpp。
#include "_ecm.hpp"
inline vector<vector<u64>> run(const vector<u64>& qs) {
 vector<vector<u64>> ans;
 ans.reserve(qs.size());
 for(u64 x: qs) ans.push_back(ecm_fact::factorize<48, true>(x));
 return ans;
}
