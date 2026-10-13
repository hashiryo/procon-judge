#pragma once
// NeoLibrary の discrete_log を呼ぶ。中身は 5 回目 (_dl5.hpp) の dl5_r22 と同じ手順 (疎な表の baby-step giant-step、r > 2^22
// で巡回群なら rho 法)。lib.hpp は今の Library の DiscreteLogarithm。
#include "../common.hpp"
#include "neo/number_theory/discrete_log.hpp"

inline vector<i64> run(const vector<array<u32, 3>>& qs) {
    vector<i64> ans;
    ans.reserve(qs.size());
    for (auto [X, Y, M] : qs) ans.push_back(discrete_log(X, Y, M));
    return ans;
}
