// harness: G 個の束「本数 分布 N の下限 N の上限 seed 引数」を読み、短い配列を全部作ってから、計測区間で run(a) を配列ごとに順に呼ぶ。
// run は a を昇順に並べ替える (self-sort-u32 と同じ形)。配列は 1 本ずつ別の vector に置く。束の中の配列は、束の seed から作る splitmix64
// の列で N を [下限, 上限] の一様分布から選び、配列の種も同じ列から取って、_shared/sort/_common.hpp の make_u32 で作る。出力は
// 「配列の本数 ハッシュ」の 1 行で、ハッシュは配列ごとの長さと hash_seq を H <- (H K + len) K + h で畳んだもの。H はどの配列の h にも
// 奇数を掛けて足した形なので、1 本の配列の誤りは hash_seq で見つかる限り H にも出る。設計は algo-notes の notes/sort-problems.md。
#include "pj.hpp"
#include "_shared/sort/_common.hpp"
#ifndef SUBMISSION_HPP
#define SUBMISSION_HPP "submissions/reference.hpp"
#endif
#include SUBMISSION_HPP

struct Group {
  unsigned long long count, lo, hi, seed, arg;
  char dist[16];
};

signed main() {
  int g;
  must_scan(scanf("%d", &g), 1);
  vector<Group> groups(g);
  size_t total = 0;
  for (auto &gr : groups) {
    must_scan(scanf("%llu %15s %llu %llu %llu %llu", &gr.count, gr.dist, &gr.lo, &gr.hi, &gr.seed, &gr.arg), 6);
    total += gr.count;
  }
  vector<vector<u32>> as;
  as.reserve(total);
  for (auto &gr : groups) {
    sort_family::SplitMix64 rng(gr.seed);
    for (unsigned long long i = 0; i < gr.count; ++i) {
      const size_t n = (size_t)(gr.lo + rng.below(gr.hi - gr.lo + 1));
      as.push_back(sort_family::make_u32(gr.dist, n, rng(), gr.arg));
    }
  }

  auto t0 = chrono::steady_clock::now();
  for (auto &a : as) run(a);
  auto t1 = chrono::steady_clock::now();

  constexpr u64 K = 0x9E3779B97F4A7C15ull;
  u64 h = 0;
  for (auto &a : as) h = (h * K + a.size()) * K + sort_family::hash_seq(a);
  printf("%zu %llu\n", as.size(), (unsigned long long)h);
  report_metrics((long long)chrono::duration_cast<chrono::nanoseconds>(t1 - t0).count());
}
