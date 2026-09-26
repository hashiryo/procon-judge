// harness: 各提出が定義する run(N, M, P, A, B) を計測する。
// yosupo の "Matrix Product" 形式の I/O。flat row-major の vector<u32> でやり取り。
#include "pj.hpp"
#include "common.hpp"

#ifndef SUBMISSION_HPP
#define SUBMISSION_HPP "submissions/naive.hpp"
#endif
#include SUBMISSION_HPP

signed main() {
    cin.tie(0);
    ios::sync_with_stdio(false);
    int N, M, P;
    cin >> N >> M >> P;
    vector<u32> a((size_t) N * M), b((size_t) M * P);
    for (auto& x : a) cin >> x;
    for (auto& x : b) cin >> x;

    constexpr int REPEAT = 1;
    uint64_t best_ns = ~uint64_t(0);
    vector<u32> result;

    for (int rep = 0; rep < REPEAT; ++rep) {
        auto t0 = chrono::steady_clock::now();
        auto r = run(N, M, P, a, b);
        auto t1 = chrono::steady_clock::now();
        result = std::move(r);
        auto ns = (uint64_t)chrono::duration_cast<chrono::nanoseconds>(t1 - t0).count();
        if (ns < best_ns) best_ns = ns;
    }

    for (int i = 0; i < N; ++i) {
        for (int j = 0; j < P; ++j) {
            cout << result[(size_t) i * P + j];
            cout << (j + 1 == P ? '\n' : ' ');
        }
    }
    report_metrics((long long)best_ns);
    return 0;
}
