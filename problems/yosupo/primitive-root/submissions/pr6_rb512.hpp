#pragma once
// 原始根の 6 回目 (核は _pr6.hpp)。8192 未満で試し割りし、残りの 2^48 以上の合成数は ECM の前に rho を 512 段まで回す。
#include "_pr6.hpp"

inline vector<u64> run(const vector<u64>& qs) {
    vector<u64> ans;
    ans.reserve(qs.size());
    for (u64 p : qs) ans.push_back(pr6::primitive_root<8192, 512>(p));
    return ans;
}
