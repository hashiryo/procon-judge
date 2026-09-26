// harness: 各提出が定義する run(N, M, A) を計測する。
// yosupo の "Rank of Matrix" 形式の I/O。
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
    vector<vector<u32>> a(N, vector<u32>(M));
    for (auto& row : a) for (auto& x : row) cin >> x;

    constexpr int REPEAT = 1;
    uint64_t best_ns = ~uint64_t(0);
    int result = 0;

    for (int rep = 0; rep < REPEAT; ++rep) {
        auto t0 = chrono::steady_clock::now();
        int r = run(N, M, a);
        auto t1 = chrono::steady_clock::now();
        result = r;
        auto ns = (uint64_t)chrono::duration_cast<chrono::nanoseconds>(t1 - t0).count();
        if (ns < best_ns) best_ns = ns;
    }

    cout << result << '\n';
    report_metrics((long long)best_ns);
    return 0;
}
