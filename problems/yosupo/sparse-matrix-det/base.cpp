// harness: 各提出が定義する run(N, K, a, b, c) を計測する。
// yosupo の "Determinant of Sparse Matrix" 形式の I/O (a_i 行 b_i 列の成分が c_i)。
#include "pj.hpp"
#include "common.hpp"

#ifndef SUBMISSION_HPP
#define SUBMISSION_HPP "submissions/lib.hpp"
#endif
#include SUBMISSION_HPP

signed main() {
    cin.tie(0);
    ios::sync_with_stdio(false);
    int N, K;
    cin >> N >> K;
    vector<int> a(K), b(K);
    vector<u32> c(K);
    for (int i = 0; i < K; ++i) cin >> a[i] >> b[i] >> c[i];

    auto t0 = chrono::steady_clock::now();
    u32 result = run(N, K, a, b, c);
    auto t1 = chrono::steady_clock::now();

    cout << result << '\n';
    report_metrics((long long)chrono::duration_cast<chrono::nanoseconds>(t1 - t0).count());
    return 0;
}
