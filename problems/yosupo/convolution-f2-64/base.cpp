// harness: 各 algos/*.hpp が定義する Solver::run(n, m, a, b) を計測する。
// 入出力フォーマット変換 (string ↔ vector<u64>) は計測外で行い、
// 純粋なアルゴリズム時間のみを ALGO_TIME_NS に含める。
#include "pj.hpp"
#include "common.hpp"

#ifndef SUBMISSION_HPP
#define SUBMISSION_HPP "submissions/naive.hpp"
#endif
#include SUBMISSION_HPP

signed main() {
 cin.tie(0);
 ios::sync_with_stdio(false);

 // 入力 parse (計測外)
 int n, m;
 std::cin >> n >> m;
 std::vector<u64> a(n), b(m);
 for (auto& x : a) std::cin >> x;
 for (auto& x : b) std::cin >> x;

 std::vector<u64> result;
 uint64_t best_ns = ~uint64_t(0);
 for (int rep = 0; rep < 1; ++rep) {
  auto t0 = chrono::steady_clock::now();
  result = Solver::run(n, m, a, b);
  auto t1 = chrono::steady_clock::now();
  auto ns = (uint64_t) chrono::duration_cast<chrono::nanoseconds>(t1 - t0).count();
  if (ns < best_ns) best_ns = ns;
 }

 // 出力 (計測外)
 for (size_t k = 0; k < result.size(); ++k) {
  std::cout << result[k];
  std::cout << (k + 1 == result.size() ? '\n' : ' ');
 }
 report_metrics((long long)best_ns);
 return 0;
}

// _shared/gf2-64/_common.hpp が開いた宣言の領域を閉じる。clang は翻訳単位の中で閉じる必要がある。
#ifdef GF2_64_TARGET_END
GF2_64_TARGET_END
#endif
