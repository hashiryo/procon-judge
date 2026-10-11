#pragma once
// 原始根の 3 回目 (核は _pr3.hpp)。最初の平方非剰余の候補で、奇数の素因数すべての g^((p-1)/q) を二分木で一度に求め、
// 満たされない q は q ごとに擬似乱数の候補で満たし、組み合わせて原始根を作る。
#include "_pr3.hpp"

inline vector<u64> run(const vector<u64>& qs) {
    vector<u64> ans;
    ans.reserve(qs.size());
    for (u64 p : qs) ans.push_back(pr3::primitive_root(p));
    return ans;
}
