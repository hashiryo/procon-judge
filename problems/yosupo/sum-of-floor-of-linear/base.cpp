// harness: 各提出が定義する run(qs) を計測する。qs[i] = {N, M, A, B}。答えは Σ_{i=0}^{N-1} ⌊(A i + B) / M⌋。
// I/O 変換は計測外で行い、純粋なアルゴリズム時間のみを ALGO_TIME_NS に含める。
#include "pj.hpp"
#include "common.hpp"

#ifndef SUBMISSION_HPP
#define SUBMISSION_HPP "submissions/lib.hpp"
#endif
#include SUBMISSION_HPP

signed main() {
 cin.tie(0);
 ios::sync_with_stdio(false);
 int T;
 cin >> T;
 vector<array<u32, 4>> qs(T);
 for(auto& q: qs) cin >> q[0] >> q[1] >> q[2] >> q[3];

 auto t0= chrono::steady_clock::now();
 vector<u64> result= run(qs);
 auto t1= chrono::steady_clock::now();

 for(auto x: result) cout << x << '\n';
 report_metrics((long long)chrono::duration_cast<chrono::nanoseconds>(t1 - t0).count());
 return 0;
}
