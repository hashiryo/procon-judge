// モノイド作用の離散対数 (自作)。入力は gen.py の説明のとおり。
//
// 提出は次を実装する。
//   struct Solver {
//     long long affine(long long p, long long a, long long b, long long s, long long t, long long N);
//     long long matrix(long long p, const array<long long, 4>& A, const array<long long, 4>& S,
//                      const array<long long, 4>& T, long long N);
//     long long permutation(const vector<int>& p, const vector<int>& s, const vector<int>& t, long long N);
//   };
// いずれも x^n s = t、0 <= n < N を満たす最小の n。無ければ -1。
// affine の p は素数とは限らない (f(x) = (a x + b) mod p)。
// matrix の p は素数で、S が初期値、T が目標 (A^n S = T、2x2 行列の積 mod p)。
// permutation は置換 p を列に作用させる (p^n(s) = t)。値は 0 .. L-1。
#include "pj.hpp"

#ifndef SUBMISSION_HPP
#define SUBMISSION_HPP "submissions/reference.hpp"
#endif
#include SUBMISSION_HPP

signed main() {
  int q;
  must_scan(scanf("%d", &q), 1);
  struct Query {
    int type;
    long long p, a, b, s, t, N;
    array<long long, 4> A, S, T;
    vector<int> perm, ps, pt;
  };
  vector<Query> qs(q);
  for (auto &e : qs) {
    must_scan(scanf("%d", &e.type), 1);
    if (e.type == 0) {
      must_scan(scanf("%lld %lld %lld %lld %lld %lld", &e.p, &e.a, &e.b, &e.s, &e.t, &e.N), 6);
    } else if (e.type == 1) {
      must_scan(scanf("%lld", &e.p), 1);
      for (auto &v : e.A) must_scan(scanf("%lld", &v), 1);
      for (auto &v : e.S) must_scan(scanf("%lld", &v), 1);
      for (auto &v : e.T) must_scan(scanf("%lld", &v), 1);
      must_scan(scanf("%lld", &e.N), 1);
    } else {
      int len;
      must_scan(scanf("%d %lld", &len, &e.N), 2);
      e.perm.resize(len), e.ps.resize(len), e.pt.resize(len);
      for (auto &v : e.perm) must_scan(scanf("%d", &v), 1);
      for (auto &v : e.ps) must_scan(scanf("%d", &v), 1);
      for (auto &v : e.pt) must_scan(scanf("%d", &v), 1);
    }
  }

  Solver sol;
  vector<long long> ans;
  ans.reserve(q);

  auto t0 = chrono::steady_clock::now();
  for (auto &e : qs) {
    if (e.type == 0) ans.push_back(sol.affine(e.p, e.a, e.b, e.s, e.t, e.N));
    else if (e.type == 1) ans.push_back(sol.matrix(e.p, e.A, e.S, e.T, e.N));
    else ans.push_back(sol.permutation(e.perm, e.ps, e.pt, e.N));
  }
  auto t1 = chrono::steady_clock::now();

  print_all(ans);
  report_metrics((long long)chrono::duration_cast<chrono::nanoseconds>(t1 - t0).count());
}
