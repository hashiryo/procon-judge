// harness: 各提出が定義する run(qs, out) を計測する。問と答えの形は common.hpp にある。
// 答えを書く場所はハーネスが確保して 0 で埋めておく (ページに触るのは計測の外)。
// I/O 変換は計測外で行い、純粋なアルゴリズム時間のみを ALGO_TIME_NS に含める。
#include "pj.hpp"
#include "common.hpp"

#ifndef SUBMISSION_HPP
#define SUBMISSION_HPP "submissions/fastest.hpp"
#endif
#include SUBMISSION_HPP

signed main() {
 cin.tie(0);
 ios::sync_with_stdio(false);
 int T;
 cin >> T;
 vector<Query> qs(T);
 for(auto& q: qs) cin >> q.a >> q.k;
 vector<u64> buf(T);

 auto t0= chrono::steady_clock::now();
 run(qs, buf.data());
 auto t1= chrono::steady_clock::now();

 string out;
 out.reserve(buf.size() * 12);
 char s[24];
 for(u64 v: buf) {
  auto r= to_chars(s, s + 24, v);
  out.append(s, r.ptr);
  out+= '\n';
 }
 fwrite(out.data(), 1, out.size(), stdout);
 report_metrics((long long)chrono::duration_cast<chrono::nanoseconds>(t1 - t0).count());
 return 0;
}
