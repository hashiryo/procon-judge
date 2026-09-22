// harness: 各 submissions/*.hpp が定義する struct DIV を使って計測する。
// 旧 judge の 1word-mod/runtime/1word-31 から移した。
// DIV は { DIV(u32 b); u32 mod(u32 a) const; } という interface を持つことを期待する。
#include "pj.hpp"
#include "../_shared/modulo-test/_common.hpp"

#ifndef SUBMISSION_HPP
#define SUBMISSION_HPP "submissions/naive.hpp"
#endif
#include SUBMISSION_HPP

signed main() {
    cin.tie(0);
    ios::sync_with_stdio(false);
    u64 n_, b_, sa_, ma_, mb_;
    cin >> n_ >> b_ >> sa_ >> ma_ >> mb_;

    constexpr u32 MASK = (u32(1) << 31) - 1;

    // 計測前に a クエリ列を全て生成しておく (生成コストを algo time から除外する)。
    vector<u32> as(n_);
    {
        u64 sa = sa_;
        for (u64 i = 0; i < n_; ++i) {
            sa = sa * ma_ + mb_;
            as[i] = u32(sa) & MASK;
        }
    }
    const u32 b = u32(b_);  // 入力で 1 <= b < 2^31 を保証する

    // REPEAT は wall time を REPEAT 倍するので TLE と相談して決める。
    constexpr int REPEAT = 1;
    uint64_t best_ns = ~uint64_t(0);
    u64 result_out = 0;

    for (int rep = 0; rep < REPEAT; ++rep) {
        const DIV div(b);  // 前計算は計測外。
        u32 acc = 0;
        auto t0 = chrono::steady_clock::now();
        for (u64 i = 0; i < n_; ++i) {
            acc ^= div.mod(as[i]);
        }
        auto t1 = chrono::steady_clock::now();
        result_out = acc;
        auto ns = (uint64_t)chrono::duration_cast<chrono::nanoseconds>(t1 - t0).count();
        if (ns < best_ns) best_ns = ns;
    }

    cout << result_out << '\n';
    report_metrics((long long)best_ns);
    return 0;
}
