// harness: T 個の (a, b) を読み、a ⊗ b ∈ GF(2^64) = GF(2)[x]/(x^64+x^4+x^3+x+1) を出力する。
// 入出力は計測区間の外。run(as, bs) だけを測る。
//
// gf2-64-mul と問題そのものは同じで、こちらは VPCLMULQDQ の 2 並列 mul とその変種を比べるための
// 場所。ループは提出が持つので、1 反復 2 積 (imm 0x00 だけ) と 1 反復 4 積 (0x00 と 0x11) の
// どちらも書ける。詰め方 (set_epi64x で組むか 256 bit 直読みするか) も提出の裁量に入る。
// テストデータを一番大きいケースでも 10^5 程度に抑えてあるのは、作業集合を cache に収めて
// 帯域ではなく mul の中身で差が付くようにするため。帯域律速の側は gf2-64-mul が見ている。
#include "pj.hpp"
#ifndef SUBMISSION_HPP
#define SUBMISSION_HPP "submissions/reference.hpp"
#endif
#include SUBMISSION_HPP

signed main() {
  int t;
  must_scan(scanf("%d", &t), 1);
  vector<u64> as(t), bs(t);
  for (int i = 0; i < t; ++i) must_scan(scanf("%llu %llu", &as[i], &bs[i]), 2);

  auto t0 = chrono::steady_clock::now();
  auto r = run(as, bs);
  auto t1 = chrono::steady_clock::now();

  print_all(r);
  report_metrics((long long)chrono::duration_cast<chrono::nanoseconds>(t1 - t0).count());
}

// _shared/gf2-64/_common.hpp が開いた宣言の領域を閉じる。clang は翻訳単位の中で閉じる必要がある。
#ifdef GF2_64_TARGET_END
GF2_64_TARGET_END
#endif
