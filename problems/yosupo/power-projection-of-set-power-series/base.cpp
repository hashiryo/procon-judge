// harness: 各提出が定義する run(N, M, a, w) を計測する。
// yosupo "Power Projection of Set Power Series" 形式の I/O。
#include "pj.hpp"
#include "common.hpp"

#ifndef SUBMISSION_HPP
#define SUBMISSION_HPP "submissions/naive.hpp"
#endif
#include SUBMISSION_HPP

signed main() {
    cin.tie(0);
    ios::sync_with_stdio(false);
    int N, M;
    cin >> N >> M;
    int sz = 1 << N;
    vector<u32> a(sz), w(sz);
    for (auto& x : a) cin >> x;
    for (auto& x : w) cin >> x;

    constexpr int REPEAT = 1;
    uint64_t best_ns = ~uint64_t(0);
    vector<u32> result;

    for (int rep = 0; rep < REPEAT; ++rep) {
        auto t0 = chrono::steady_clock::now();
        auto r = run(N, M, a, w);
        auto t1 = chrono::steady_clock::now();
        result = std::move(r);
        auto ns = (uint64_t)chrono::duration_cast<chrono::nanoseconds>(t1 - t0).count();
        if (ns < best_ns) best_ns = ns;
    }

    for (int i = 0; i < M; ++i) {
        cout << result[i];
        cout << (i + 1 == M ? '\n' : ' ');
    }
    report_metrics((long long)best_ns);
    return 0;
}
