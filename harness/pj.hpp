#pragma once
// すべての問題のハーネスと提出が共通で使うもの。
// bits/stdc++.h は Apple clang に無いので、必要なものを名指しで include する。
#include <array>
#include <cctype>
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <vector>

#if !defined(__linux__)
#include <sys/resource.h>
#endif

using namespace std;

// scanf の %lld と揃えたいので int64_t ではなく long long を使う。
using i64 = long long;
using u64 = unsigned long long;
using u32 = unsigned int;

// 読み取れた項目数が想定と違ったら落とす。黙って進むと WA の原因が見えない。
inline void must_scan(int got, int want) {
  if (got != want) {
    fprintf(stderr, "input format error\n");
    exit(1);
  }
}

// 空白で区切られた次のトークンを読む。長さが分からない入力 (文字列) 用。
// scanf と同じ stdin を読むので、混ぜて使ってよい。
inline string read_token() {
  string s;
  int c = getchar();
  while (c != EOF && isspace(c)) c = getchar();
  if (c == EOF) {
    fprintf(stderr, "input format error\n");
    exit(1);
  }
  while (c != EOF && !isspace(c)) {
    s.push_back((char)c);
    c = getchar();
  }
  return s;
}

// 整数を n 個読む。
inline vector<i64> read_ints(int n) {
  vector<i64> a(n);
  for (auto &x : a) must_scan(scanf("%lld", &x), 1);
  return a;
}

// 数の列を書く。既定は 1 行 1 個で、sep を渡すと区切りを変えられる。
// 末尾は sep に関わらず改行にする。空なら何も書かない。
//
// 整形は計測区間の外に置きたいので、いったん文字列に溜めてから 1 回で書く。
template <class T> inline void print_all(const vector<T> &v, char sep = '\n') {
  if (v.empty()) return;
  string out;
  out.reserve(v.size() * 12);
  for (size_t i = 0; i + 1 < v.size(); ++i) {
    out += to_string(v[i]);
    out += sep;
  }
  out += to_string(v.back());
  out += '\n';
  fwrite(out.data(), 1, out.size(), stdout);
}

// ピーク RSS を KB で返す。取れなければ -1。
//
// 実行側の ru_maxrss は使えない。posix_spawn した子のそれには親のピークが
// 混ざる。カーネルが exec のときに古い mm の high-water を引き継ぐためで、
// pj 自身の RSS がそのまま下駄になる。Linux の /proc/self/status の VmHWM は
// exec 後の mm だけを見るので汚れない。
inline long long peak_rss_kb() {
#if defined(__linux__)
  FILE *f = fopen("/proc/self/status", "r");
  if (!f) return -1;
  char line[256];
  long long kb = -1;
  while (fgets(line, sizeof line, f))
    if (sscanf(line, "VmHWM: %lld kB", &kb) == 1) break;
  fclose(f);
  return kb;
#else
  // macOS の posix_spawn はアドレス空間を共有しないので ru_maxrss で足りる。
  // ただし単位はバイト。
  rusage ru;
  if (getrusage(RUSAGE_SELF, &ru) != 0) return -1;
  return (long long)ru.ru_maxrss / 1024;
#endif
}

// 計測値は stderr に 1 行の JSON で出す。実行側はこの接頭辞の行だけを拾う。
// メモリは出力の整形まで終わってから読むので、main の末尾で呼ぶこと。
inline void report_metrics(long long algo_time_ns) {
  fprintf(stderr, "PJ_METRICS {\"algo_time_ns\":%lld,\"max_rss_kb\":%lld}\n",
          algo_time_ns, peak_rss_kb());
}
