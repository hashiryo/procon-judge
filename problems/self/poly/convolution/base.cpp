// 2 つの列の畳み込み c_k = Σ_{i+j=k} a_i b_j (mod 998244353) を、ケースごとに求める。長さは 8192 以下で、FFT を使わない
// 素朴な積で収まる大きさ。短い列どうしを大量に掛ける入力と、片方だけ長い入力もある。
//
// 提出は次を実装する。
//   vector<u32> run(const vector<u32> &a, const vector<u32> &b);  // 長さ |a| + |b| - 1
// 計測は全ケースの run の時間の和。
#include "pj.hpp"
#ifndef SUBMISSION_HPP
#define SUBMISSION_HPP "submissions/reference.hpp"
#endif
#include SUBMISSION_HPP
signed main() {
  int t;
  must_scan(scanf("%d", &t), 1);
  vector<vector<u32>> as(t), bs(t);
  for (int q = 0; q < t; ++q) {
    int n, m;
    must_scan(scanf("%d %d", &n, &m), 2);
    as[q].resize(n), bs[q].resize(m);
    for (auto &x : as[q]) must_scan(scanf("%u", &x), 1);
    for (auto &x : bs[q]) must_scan(scanf("%u", &x), 1);
  }
  vector<vector<u32>> cs(t);
  auto t0 = chrono::steady_clock::now();
  for (int q = 0; q < t; ++q) cs[q] = run(as[q], bs[q]);
  auto t1 = chrono::steady_clock::now();
  string out;
  for (auto &c : cs) {
    for (size_t i = 0; i < c.size(); ++i) {
      out += to_string(c[i]);
      out += i + 1 == c.size() ? '\n' : ' ';
    }
  }
  fputs(out.c_str(), stdout);
  report_metrics((long long)chrono::duration_cast<chrono::nanoseconds>(t1 - t0).count());
}
