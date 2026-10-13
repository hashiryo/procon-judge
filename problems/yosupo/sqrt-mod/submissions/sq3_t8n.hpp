#pragma once
// 平方根 mod p の 3 回目 (核は _sq3.hpp)。P - 1 の 2 の冪が 8 より大きいときは Lucas 列の梯子で、(Y / P) を先に 1 本で求め、候補の a を 2 つずつ並べる。
#include "_sq3.hpp"

inline vector<i64> run(const vector<array<u32, 2>>& qs) {
    vector<i64> ans;
    ans.reserve(qs.size());
    for (auto [Y, P] : qs) ans.push_back(sq3::sqrt_mod<8, false>(Y, P));
    return ans;
}
