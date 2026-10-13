// harness: 各提出が定義する run(qs) を計測する。qs[i] = {X, Y, M}。答えは X^K ≡ Y (mod M) となる最小の K (無ければ -1)。
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
 vector<array<u32, 3>> qs(T);
 for(auto& q: qs) cin >> q[0] >> q[1] >> q[2];

 auto t0= chrono::steady_clock::now();
 vector<i64> result= run(qs);
 auto t1= chrono::steady_clock::now();

 for(auto x: result) cout << x << '\n';
 report_metrics((long long)chrono::duration_cast<chrono::nanoseconds>(t1 - t0).count());
 return 0;
}
