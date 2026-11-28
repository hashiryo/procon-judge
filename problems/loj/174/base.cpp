// loj-174: Binomial Convolution。n、m、p と a_0..a_n、b_0..b_m が与えられ、
// c_k = Σ_i C(k, i) a_i b_{k-i} (mod p) を c_0..c_{n+m} として出す (n, m <= 10^5、p <= 10^9、p は素数とは限らない)。
// ハーネスの口は self/ntt の二項畳み込みの問題と同じ。
// 入出力は pj.hpp の scanf と printf で行う (提出が iostream を include するかどうかに寄らないため)。
//
// 提出は struct Kernel に次を実装する。
//   static void convolve(const u32* a, int n, const u32* b, int m, u32* c, u32 mod);
//   // c[k] = Σ_i C(k, i) a[i] b[k-i] (0 ≤ k < n + m - 1)。c は a、b と重ならない。a、b の値は mod 未満
#include "pj.hpp"

#ifndef SUBMISSION_HPP
#define SUBMISSION_HPP "submissions/reference.hpp"
#endif
#include SUBMISSION_HPP

signed main() {
 int n, m;
 unsigned p;
 must_scan(scanf("%d %d %u", &n, &m, &p), 3);
 vector<u32> a(n + 1), b(m + 1), c(n + m + 1);
 for (auto& x: a) must_scan(scanf("%u", &x), 1);
 for (auto& x: b) must_scan(scanf("%u", &x), 1);

 auto t0= chrono::steady_clock::now();
 Kernel::convolve(a.data(), n + 1, b.data(), m + 1, c.data(), p);
 auto t1= chrono::steady_clock::now();

 for (size_t i= 0; i < c.size(); ++i) printf(i ? " %u" : "%u", c[i]);
 putchar('\n');
 report_metrics((long long)chrono::duration_cast<chrono::nanoseconds>(t1 - t0).count());
 return 0;
}
