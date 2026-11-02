// harness: 各提出が定義する run(N, A) を計測する。
// yosupo の "Hafnian of Matrix" 形式の I/O。
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

    auto t0 = chrono::steady_clock::now();
    u32 result = run(N, a);
    auto t1 = chrono::steady_clock::now();

    cout << result << '\n';
    report_metrics((long long)chrono::duration_cast<chrono::nanoseconds>(t1 - t0).count());
    return 0;
}
