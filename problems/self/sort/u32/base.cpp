// harness: T 個の配列の作り方「分布 N seed 引数」を読み、配列を作ってから、計測区間で run(a) を順に呼ぶ。
// run は a を昇順に並べ替える。作業用の vector と中身を入れ替えて返してもよい。作業用の配列の確保と、
// それを初めて触るときのページフォールトも計測区間に入る。出力は配列ごとに「長さ ハッシュ」の 1 行。
// 配列の作り方とハッシュは _shared/sort/_common.hpp にある。設計は algo-notes の notes/sort-problems.md。
#include "pj.hpp"
#include "_shared/sort/_common.hpp"
#ifndef SUBMISSION_HPP
#define SUBMISSION_HPP "submissions/reference.hpp"
#endif
#include SUBMISSION_HPP

signed main() {
  int t;
  must_scan(scanf("%d", &t), 1);
  vector<vector<u32>> as(t);
  for (auto &a : as) {
    char dist[16];
    unsigned long long n, seed, arg;
    must_scan(scanf("%15s %llu %llu %llu", dist, &n, &seed, &arg), 4);
    a = sort_family::make_u32(dist, (size_t)n, seed, arg);
  }

  auto t0 = chrono::steady_clock::now();
  for (auto &a : as) run(a);
  auto t1 = chrono::steady_clock::now();

  string out;
  for (auto &a : as) out += to_string(a.size()) + ' ' + to_string(sort_family::hash_seq(a)) + '\n';
  fwrite(out.data(), 1, out.size(), stdout);
  report_metrics((long long)chrono::duration_cast<chrono::nanoseconds>(t1 - t0).count());
}
