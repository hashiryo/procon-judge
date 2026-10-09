// harness: T 個の配列の作り方「分布 N seed 引数」を読み、配列を作ってから、計測区間で xs = run(a) を順に呼ぶ。
// run は a の各要素を順位 (a の中でその要素より小さい値の種類数、0 始まり) に置き換え、重複を除いて昇順に並べた値の列 xs を返す。
// 順位は i64 のまま a に書く。run のあとは xs[a[i]] が元の a[i] になり、答えは 1 つに決まる。a は作業用の vector と中身を入れ替えても
// よい。返り値の xs と作業用の配列の確保も計測区間に入る。出力は配列ごとに「N a のハッシュ K xs のハッシュ」の 1 行 (K は xs の長さ)。
// 配列の作り方とハッシュは self-sort-i64 と同じ _shared/sort/_common64.hpp にある。設計は algo-notes の notes/sort-problems.md。
#include "pj.hpp"
#include "_shared/sort/_common64.hpp"
#ifndef SUBMISSION_HPP
#define SUBMISSION_HPP "submissions/reference.hpp"
#endif
#include SUBMISSION_HPP

signed main() {
  int t;
  must_scan(scanf("%d", &t), 1);
  vector<vector<i64>> as(t);
  for (auto &a : as) {
    char dist[16];
    unsigned long long n, seed, arg;
    must_scan(scanf("%15s %llu %llu %llu", dist, &n, &seed, &arg), 4);
    a = sort_family::make_i64(dist, (size_t)n, seed, arg);
  }
  vector<vector<i64>> xss(t);

  auto t0 = chrono::steady_clock::now();
  for (int i = 0; i < t; ++i) xss[i] = run(as[i]);
  auto t1 = chrono::steady_clock::now();

  string out;
  for (int i = 0; i < t; ++i) {
    out += to_string(as[i].size()) + ' ' + to_string(sort_family::hash_seq64(as[i])) + ' ';
    out += to_string(xss[i].size()) + ' ' + to_string(sort_family::hash_seq64(xss[i])) + '\n';
  }
  fwrite(out.data(), 1, out.size(), stdout);
  report_metrics((long long)chrono::duration_cast<chrono::nanoseconds>(t1 - t0).count());
}
