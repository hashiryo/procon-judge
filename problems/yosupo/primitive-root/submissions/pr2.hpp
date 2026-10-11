#pragma once
// 原始根の 2 回目 (核は _pr2.hpp)。候補ごとに奇数の素因数すべての g^((p-1)/q) を二分木で一度に求め、1 つの候補で
// 足りなければ、満たす q の違う候補を組み合わせて原始根を作る。
#include "_pr2.hpp"

inline vector<u64> run(const vector<u64>& qs) {
    vector<u64> ans;
    ans.reserve(qs.size());
    for (u64 p : qs) ans.push_back(pr2::primitive_root(p));
    return ans;
}
