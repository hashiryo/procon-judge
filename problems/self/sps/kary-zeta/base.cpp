// k 進の桁ごとのゼータ変換とメビウス変換 (自作)。入力は gen.py の説明のとおり。
//
// 提出は次を実装する。
//   struct Solver {
//     Solver(int t, int n, const vector<i64> &a);  // 構築。計測区間の外。
//     void run();                                  // 変換。ここだけ測る。
//     vector<i64> answer() const;                  // k^n 個
//   };
//
// 添字 x を k 進の n 桁と見る。y ≤ x は桁ごとに y の桁が x の桁以下のこと。t は k と変換と値の型を決める。
//   0: k = 10、f[x] <- sum_{y ≤ x} f[y]。値は long long。
//   1: k = 3、f[x] <- max_{y ≥ x} f[y]。値は int。
//   2: k = 4、0 の逆 (f[x] = sum_{y ≤ x} g[y] となる g)。値は mod 998244353。
//   3: k = 5、f[x] <- sum_{y ≥ x} f[y]。値は unsigned long long で 2^64 を法として足し、答えは i64 に読み替えて出す。
//   4: k = 7、f[x] <- min_{y ≤ x} f[y]。値は long long。
#include "pj.hpp"

#ifndef SUBMISSION_HPP
#define SUBMISSION_HPP "submissions/reference.hpp"
#endif
#include SUBMISSION_HPP

static constexpr int KARY_K[5] = {10, 3, 4, 5, 7};

signed main() {
  int t, n;
  must_scan(scanf("%d %d", &t, &n), 2);
  int N = 1;
  for (int i = 0; i < n; ++i) N *= KARY_K[t];
  vector<i64> a = read_ints(N);

  Solver sol(t, n, a);

  auto t0 = chrono::steady_clock::now();
  sol.run();
  auto t1 = chrono::steady_clock::now();

  print_all(sol.answer(), ' ');

  // 出力の整形まで含めたピークを読みたいので、計測値は最後に出す。
  report_metrics(
      (long long)chrono::duration_cast<chrono::nanoseconds>(t1 - t0).count());
}
