// harness: 各 submissions/*.hpp が定義する struct MP を使って計測する
// 旧 judge の modulo-test/runtime/runtime-32 から移した。
#include "pj.hpp"
#include "../_shared/modulo-test/_common.hpp"

#ifndef SUBMISSION_HPP
#define SUBMISSION_HPP "submissions/naive.hpp"
#endif
#include SUBMISSION_HPP

signed main() {
    cin.tie(0);
    ios::sync_with_stdio(false);
    u64 n_, mod_, state_, a_, b_, c_, d_;
    cin >> n_ >> mod_ >> state_ >> a_ >> b_ >> c_ >> d_;

    // REPEAT は wall time を REPEAT 倍するので TLE と相談して決める。
    constexpr int REPEAT = 1;
    uint64_t best_ns = ~uint64_t(0);
    u64 result_out = 0;

    for (int rep = 0; rep < REPEAT; ++rep) {
    const MP mp(mod_);
        auto state = mp.set(state_);
        auto a = mp.set(a_);
        auto b = mp.set(b_);
        auto c = mp.set(c_);
        auto d = mp.set(d_);
        auto t0 = chrono::steady_clock::now();
        for (u64 i = 0; i < n_; ++i) {
            state = mp.mul(mp.plus(mp.mul(state, a), b), mp.plus(mp.mul(state, c), d));
        }
        auto t1 = chrono::steady_clock::now();
        result_out = mp.get(state);
        auto ns = (uint64_t)chrono::duration_cast<chrono::nanoseconds>(t1 - t0).count();
        if (ns < best_ns) best_ns = ns;
    }

    cout << result_out << '\n';
    report_metrics((long long)best_ns);
    return 0;
}
