// CodeChef DANYANUM (Danya and Numbers)。数の追加と削除をしながら、長さ x の部分列の AND の最大を問う。
//
// 提出は次を実装する。操作は {種類, v}。種類 1 は v を足す、2 は v を 1 つ消す、3 は f(v, A) を問う。
//   struct Solver {
//     Solver(int k, const vector<int> &a, const vector<pair<int, int>> &ops);  // 構築。計測区間の外。
//     void run();                                                              // ここだけ測る。
//     vector<int> answer() const;                                              // 種類 3 の答えを順に
//   };
#include "pj.hpp"

#ifndef SUBMISSION_HPP
#define SUBMISSION_HPP "submissions/reference.hpp"
#endif
#include SUBMISSION_HPP

signed main() {
  int n, m, k;
  must_scan(scanf("%d %d %d", &n, &m, &k), 3);
  vector<int> a(n);
  for (auto &x : a) must_scan(scanf("%d", &x), 1);
  vector<pair<int, int>> ops(m);
  for (auto &[t, v] : ops) must_scan(scanf("%d %d", &t, &v), 2);

  Solver sol(k, a, ops);
  auto t0 = chrono::steady_clock::now();
  sol.run();
  auto t1 = chrono::steady_clock::now();

  const vector<int> r = sol.answer();
  string out;
  out.reserve(r.size() * 8);
  char buf[16];
  for (int x : r) {
    const int len = snprintf(buf, sizeof(buf), "%d\n", x);
    out.append(buf, len);
  }
  fwrite(out.data(), 1, out.size(), stdout);
  report_metrics((long long)chrono::duration_cast<chrono::nanoseconds>(t1 - t0).count());
}
