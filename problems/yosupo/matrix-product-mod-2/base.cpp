// harness: 各提出が定義する run(N, M, K, A, B) を計測する。
// yosupo の "Matrix Product (mod 2)" 形式の I/O。
#include "pj.hpp"
#include "common.hpp"

#ifndef SUBMISSION_HPP
#define SUBMISSION_HPP "submissions/naive.hpp"
#endif
#include SUBMISSION_HPP

signed main() {
    cin.tie(0);
    ios::sync_with_stdio(false);
    int N, M, K;
    cin >> N >> M >> K;
    vector<string> a(N), b(M);
    for (auto& s : a) cin >> s;
    for (auto& s : b) cin >> s;

    constexpr int REPEAT = 1;
    uint64_t best_ns = ~uint64_t(0);
    vector<string> result;

    for (int rep = 0; rep < REPEAT; ++rep) {
        auto t0 = chrono::steady_clock::now();
        auto r = run(N, M, K, a, b);
        auto t1 = chrono::steady_clock::now();
        result = std::move(r);
        auto ns = (uint64_t)chrono::duration_cast<chrono::nanoseconds>(t1 - t0).count();
        if (ns < best_ns) best_ns = ns;
    }

    for (const auto& s : result) cout << s << '\n';
    report_metrics((long long)best_ns);
    return 0;
}
