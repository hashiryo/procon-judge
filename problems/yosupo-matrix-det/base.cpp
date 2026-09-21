// harness: 各 algos/*.hpp が定義する struct Det::run(N, A) を計測する。
// yosupo の "Determinant of Matrix" 形式の I/O。
#include "pj.hpp"
#include "common.hpp"

#ifndef SUBMISSION_HPP
#define SUBMISSION_HPP "submissions/naive.hpp"
#endif
#include SUBMISSION_HPP

signed main() {
    cin.tie(0);
    ios::sync_with_stdio(false);
    int N;
    cin >> N;
    vector<vector<u32>> a(N, vector<u32>(N));
    for (auto& row : a) for (auto& x : row) cin >> x;

    constexpr int REPEAT = 1;
    uint64_t best_ns = ~uint64_t(0);
    u32 result = 0;

    for (int rep = 0; rep < REPEAT; ++rep) {
        auto t0 = chrono::steady_clock::now();
        u32 r = Det::run(N, a);
        auto t1 = chrono::steady_clock::now();
        result = r;
        auto ns = (uint64_t)chrono::duration_cast<chrono::nanoseconds>(t1 - t0).count();
        if (ns < best_ns) best_ns = ns;
    }

    cout << result << '\n';
    report_metrics((long long)best_ns);
    return 0;
}
