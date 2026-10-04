// harness: T 組の (a, b) と K を読み、組ごとに a と b をそれぞれ K 回二乗した a^(2^K) と b^(2^K) を出力する
// (GF(2^64) = GF(2)[x]/(x^64+x^4+x^3+x+1))。入出力は計測区間の外で、run だけを測る。
//
// log-any のように 2 つの値を __m256i の 2 lane に並べて計算するときの、2 並列の二乗を比べる場所。
// 提出は sq2 だけを書く。sq2(v) は、v の qword 0 と 2 にある 2 つの値の二乗を、同じ qword に入れて
// 返す。qword 1 と 3 は読まなくてよく、返す値のそこも何でもよい。mul2 と同じ並びなので、答えを
// そのまま mul2 に渡せる。
//
// ループはこのハーネスが持ち、組ごとに v = sq2(v) を K 回続ける。どの回も前の答えを待つので、
// 測るのは sq2 の待ち時間 (latency) になる。スカラの sq を 2 回呼ぶ提出は、lane から値を取り出して
// 二乗し、__m256i に詰め直すところまでを sq2 の中で払う。log-any で mul2 の答えを二乗して次の
// mul2 に渡すところと同じ形にするため。独立な二乗をたくさん流す速さは gf2-64-sq が見ている。
#include "pj.hpp"
#ifndef SUBMISSION_HPP
#define SUBMISSION_HPP "submissions/reference.hpp"
#endif
#include SUBMISSION_HPP

vector<u64> run(const vector<u64>& as, const vector<u64>& bs, int k) {
  // 1 回ごとに挟む、中身は恒等の並べ替え。添字を実行時にしか分からない値から作る。これが無いと、
  // コンパイラは次の回の取り出しと前の回の詰め直しを打ち消し、スカラの sq を 2 回呼ぶ提出を
  // スカラのまま回してしまう (log-any では前後が mul2 なので、そうはならない)。pshufb が 1 回ぶん
  // 待ちに乗るが、どの提出も同じだけ払う。
  volatile char zero = 0;
  const __m256i perm = _mm256_add_epi8(
      _mm256_setr_epi8(0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15),
      _mm256_set1_epi8(zero));
  const size_t n = as.size();
  vector<u64> ans(2 * n);
  for (size_t i = 0; i < n; ++i) {
    __m256i v = _mm256_set_epi64x(0, (long long)bs[i], 0, (long long)as[i]);
    for (int j = 0; j < k; ++j) v = _mm256_shuffle_epi8(sq2(v), perm);
    ans[2 * i] = u64(_mm256_extract_epi64(v, 0));
    ans[2 * i + 1] = u64(_mm256_extract_epi64(v, 2));
  }
  return ans;
}

signed main() {
  int t, k;
  must_scan(scanf("%d %d", &t, &k), 2);
  vector<u64> as(t), bs(t);
  for (int i = 0; i < t; ++i) must_scan(scanf("%llu %llu", &as[i], &bs[i]), 2);

  auto t0 = chrono::steady_clock::now();
  auto r = run(as, bs, k);
  auto t1 = chrono::steady_clock::now();

  print_all(r);
  report_metrics((long long)chrono::duration_cast<chrono::nanoseconds>(t1 - t0).count());
}

// _shared/gf2-64/_common.hpp が開いた宣言の領域を閉じる。clang は翻訳単位の中で閉じる必要がある。
#ifdef GF2_64_TARGET_END
GF2_64_TARGET_END
#endif
