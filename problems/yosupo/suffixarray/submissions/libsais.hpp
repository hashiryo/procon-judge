#pragma once
// libsais 2.10.4 (https://github.com/IlyaGrebnov/libsais、Ilya Grebnov、Apache License 2.0) を、届く先の目安として呼ぶ提出。libsais は
// 種類の配列を持たず、バケットごとに 2 本のポインタを持って induced sorting の走査を分岐なしにし、LMS の部分文字列の順位も induce の
// 中で付け、一度しか出ない LMS の部分文字列を再帰の文字列から外す (README の Algorithm の節)。v2.10.4 の src/libsais.c、
// include/libsais.h、LICENSE を、手を入れずに problems/_shared/libsais/ に置いた。OpenMP は使わず (LIBSAIS_OPENMP を定義しない)
// 1 スレッドで、配列の後ろの空き fs は 0 (README で多くの場合に足りるとされる値) にする。
// 接尾辞配列の実装を比べた記録は algo-notes の notes/suffix-array.md にある。
#include <cstdint>
#include <vector>
#include "pj.hpp"
#include "_shared/libsais/libsais.c"

struct Solver {
  string s;
  vector<int> sa;

  explicit Solver(const string &s) : s(s) {}

  void run() {
    sa.resize(s.size());
    libsais(reinterpret_cast<const uint8_t *>(s.data()), sa.data(), int32_t(s.size()), 0, nullptr);
  }

  const vector<int> &answer() const { return sa; }
};
