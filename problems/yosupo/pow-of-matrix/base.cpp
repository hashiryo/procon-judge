// harness: 各提出が定義する run(N, K, A) を計測する。
// yosupo "Pow of Matrix" 形式の I/O。行列は行優先の 1 本の vector<u32> でやり取りする。
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
    u64 K;
    cin >> N >> K;
    vector<u32> a((size_t) N * N);
    for (auto& x : a) cin >> x;

    auto t0 = chrono::steady_clock::now();
    vector<u32> result = run(N, K, a);
    auto t1 = chrono::steady_clock::now();
    auto ns = (uint64_t)chrono::duration_cast<chrono::nanoseconds>(t1 - t0).count();

    string out;
    out.reserve((size_t) N * N * 11);
    char buf[16];
    for (int i = 0; i < N; ++i) {
        for (int j = 0; j < N; ++j) {
            auto [p, ec] = to_chars(buf, buf + 16, result[(size_t) i * N + j]);
            out.append(buf, p);
            out += (j + 1 == N ? '\n' : ' ');
        }
    }
    fputs(out.c_str(), stdout);
    report_metrics((long long)ns);
    return 0;
}
