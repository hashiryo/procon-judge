#pragma once
// 平方根 mod p の 4 回目 (核は _sq4.hpp)。Jacobi 記号の段は差の ctz を取り、x86 では cmov で書く。(Y / P) と (1 - Y / P) を並べて求め、梯子の選択をビットの分岐にする。
#include "_sq4.hpp"

inline vector<i64> run(const vector<array<u32, 2>>& qs) {
    vector<i64> ans;
    ans.reserve(qs.size());
    for (auto [Y, P] : qs) ans.push_back(sq4::sqrt_mod<8, true, true, true>(Y, P));
    return ans;
}
