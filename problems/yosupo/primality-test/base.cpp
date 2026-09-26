// harness: 各提出が定義する run(qs) を計測する。
// yosupo "Primality Test" 形式の I/O。
#include "pj.hpp"
#include "common.hpp"

#ifndef SUBMISSION_HPP
#define SUBMISSION_HPP "submissions/naive.hpp"
#endif
#include SUBMISSION_HPP

signed main() {
    cin.tie(0);
    ios::sync_with_stdio(false);
    int Q;
    cin >> Q;
    vector<u64> qs(Q);
    for (auto& x : qs) cin >> x;

    constexpr int REPEAT = 1;
    uint64_t best_ns = ~uint64_t(0);
    vector<bool> result;

    for (int rep = 0; rep < REPEAT; ++rep) {
        auto t0 = chrono::steady_clock::now();
        auto r = run(qs);
        auto t1 = chrono::steady_clock::now();
        result = std::move(r);
        auto ns = (uint64_t)chrono::duration_cast<chrono::nanoseconds>(t1 - t0).count();
        if (ns < best_ns) best_ns = ns;
    }

    for (int i = 0; i < Q; ++i) cout << (result[i] ? "Yes" : "No") << '\n';
    report_metrics((long long)best_ns);
    return 0;
}
