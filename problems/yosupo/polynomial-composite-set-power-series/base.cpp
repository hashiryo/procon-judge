// harness: 各提出が定義する Solver の run() だけを計測する。
// yosupo "Polynomial Composite Set Power Series" 形式の I/O。
//
// 提出は次を実装する。
//   struct Solver {
//     Solver(int M, int N, const vector<u32> &f, const vector<u32> &b);  // 構築。計測区間の外 (値の型への写しなど)。
//     void run();                 // f(b) を求める。ここだけ測る。
//     vector<u32> answer() const;  // 答え。計測区間の外。
//   };
// 2026-11-17 に、run(...) の呼び出しを測る形から替えた。ライブラリの値の型 (ZMod、ModInt) への写しと戻しが
// 計測に入らないようにするため (algo-notes の notes/set-power-series.md)。
#include "pj.hpp"
#include "common.hpp"

#ifndef SUBMISSION_HPP
#define SUBMISSION_HPP "submissions/naive.hpp"
#endif
#include SUBMISSION_HPP

signed main() {
    cin.tie(0);
    ios::sync_with_stdio(false);
    int M, N;
    cin >> M >> N;
    vector<u32> f(M);
    for (auto& x : f) cin >> x;
    int sz = 1 << N;
    vector<u32> b(sz);
    for (auto& x : b) cin >> x;

    Solver sol(M, N, f, b);
    auto t0 = chrono::steady_clock::now();
    sol.run();
    auto t1 = chrono::steady_clock::now();
    const vector<u32> result = sol.answer();
    const uint64_t best_ns = (uint64_t)chrono::duration_cast<chrono::nanoseconds>(t1 - t0).count();

    for (int i = 0; i < sz; ++i) {
        cout << result[i];
        cout << (i + 1 == sz ? '\n' : ' ');
    }
    report_metrics((long long)best_ns);
    return 0;
}
