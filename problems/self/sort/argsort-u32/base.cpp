// harness: T 個の配列の作り方「分布 N seed 引数」を読み、配列を作ってから、計測区間で p = run(a) を順に呼ぶ。
// run は a[p[0]] <= a[p[1]] <= ... となる添字の列 p を返す。等しい値は添字の小さい順 (安定な並べ方) で、答えは 1 つに決まる。
// a は読むだけで書き換えない。返り値の vector と作業用の配列の確保も計測区間に入る。出力は配列ごとに「p の長さ ハッシュ」の 1 行。
// 配列の作り方とハッシュは self-sort-u32 と同じ _shared/sort/_common.hpp にある。設計は algo-notes の notes/sort-problems.md。
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
  vector<vector<u32>> ps(t);

  auto t0 = chrono::steady_clock::now();
  for (int i = 0; i < t; ++i) ps[i] = run(as[i]);
  auto t1 = chrono::steady_clock::now();

  string out;
  for (auto &p : ps) out += to_string(p.size()) + ' ' + to_string(sort_family::hash_seq(p)) + '\n';
  fwrite(out.data(), 1, out.size(), stdout);
  report_metrics((long long)chrono::duration_cast<chrono::nanoseconds>(t1 - t0).count());
}
