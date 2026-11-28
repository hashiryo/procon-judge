// 法 998244353 固定の階乗と二項係数 (自作)。入力は gen.py の説明のとおり。
//
// 提出は次を実装する。
//   struct Solver { std::vector<unsigned> run(unsigned N, const std::vector<unsigned>& ns, const std::vector<unsigned>& rs); };
// 各 i について n_i C r_i mod 998244353 を返す。0 <= r_i <= n_i <= N < 998244353。
// 表の構築 (N! まで) と問いの両方を run の中でやる。
#include "pj.hpp"
#include "common.hpp"

#ifndef SUBMISSION_HPP
#define SUBMISSION_HPP "submissions/reference.hpp"
#endif
#include SUBMISSION_HPP

signed main() {
 unsigned N;
 int q;
 must_scan(scanf("%u %d", &N, &q), 2);
 vector<u32> ns(q), rs(q);
 for (int i = 0; i < q; ++i) must_scan(scanf("%u %u", &ns[i], &rs[i]), 2);

 Solver sol;
 auto t0 = chrono::steady_clock::now();
 auto ans = sol.run(N, ns, rs);
 auto t1 = chrono::steady_clock::now();

 print_all(ans);
 report_metrics((long long)chrono::duration_cast<chrono::nanoseconds>(t1 - t0).count());
}
