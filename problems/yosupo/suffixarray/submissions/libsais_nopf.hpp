#pragma once
// libsais の提出 (libsais.hpp) から、先読みだけを抜いた診断用の版。libsais との差のうち、先読みの分を見る。libsais.c は GCC と Clang
// では先読みを __builtin_prefetch で頼むので、_libsais/libsais.c を読む間だけ __builtin_prefetch を何もしない形に置き換える (先読みの
// 番地の式は残るが、使われないので消える)。_libsais/ のファイルには手を入れない。ほかは libsais.hpp と同じ。以下は libsais.hpp の説明。
// libsais 2.10.4 (https://github.com/IlyaGrebnov/libsais、Ilya Grebnov、Apache License 2.0) を、届く先の目安として呼ぶ提出。libsais は
// 種類の配列を持たず、バケットごとに 2 本のポインタを持って induced sorting の走査を分岐なしにし、LMS の部分文字列の順位も induce の
// 中で付け、一度しか出ない LMS の部分文字列を再帰の文字列から外す (README の Algorithm の節)。v2.10.4 の src/libsais.c、
// include/libsais.h、LICENSE を、手を入れずに _libsais/ に置いた。OpenMP は使わず (LIBSAIS_OPENMP を定義しない) 1 スレッドで、配列の
// 後ろの空き fs は 0 (README で多くの場合に足りるとされる値) にする。
// 接尾辞配列の実装を比べた記録は algo-notes の notes/suffix-array.md にある。
#include <cstdint>
#include <vector>
#include "pj.hpp"
#define __builtin_prefetch(address, ...) ((void)(address))
#include "_libsais/libsais.c"
#undef __builtin_prefetch

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
