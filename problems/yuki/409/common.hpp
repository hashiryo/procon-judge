#pragma once
// ライブラリを使う提出が共有するもの。費用関数はどちらも同じなので、ここに
// 置いて解き方だけを変える。
#include "pj.hpp"

// 最後に i 日目から N 日目まで運動を続けたときの増減。
inline i64 tail(i64 a, i64 b, i64 n, i64 i) {
  return b * (n - i) * (n - i + 1) / 2 - a * (n - i);
}
