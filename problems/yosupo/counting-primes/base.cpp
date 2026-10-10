// harness: 各提出が定義する run(N) を計測する。
// yosupo "Counting Primes" 形式の I/O。
#include "pj.hpp"
#include "common.hpp"

#ifndef SUBMISSION_HPP
#define SUBMISSION_HPP "submissions/lib.hpp"
#endif
#include SUBMISSION_HPP

signed main() {
    cin.tie(0);
    ios::sync_with_stdio(false);
    u64 N;
    cin >> N;

    constexpr int REPEAT = 1;
    uint64_t best_ns = ~uint64_t(0);
    u64 result = 0;

    for (int rep = 0; rep < REPEAT; ++rep) {
        auto t0 = chrono::steady_clock::now();
        u64 r = run(N);
        auto t1 = chrono::steady_clock::now();
        result = r;
        auto ns = (uint64_t)chrono::duration_cast<chrono::nanoseconds>(t1 - t0).count();
        if (ns < best_ns) best_ns = ns;
    }

    cout << result << '\n';
    report_metrics((long long)best_ns);
    return 0;
}
