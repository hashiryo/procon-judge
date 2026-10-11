#pragma once
// 文字ごとの min-plus の行列。状態 j は "abc" の先頭 j 文字が部分列として現れていること (j = 0, 1, 2)。行 i から列 j へ
// 移る費用を M(i, j) に置き、行ベクトル v を v' = v M (v'_j = min_i v_i + M(i, j)) で進める。
//   'a': 0 -> 1 は 0 (そのまま)、0 -> 0 は 1 (書き換える)、1 -> 1 と 2 -> 2 は 0
//   'b': 1 -> 2 は 0、1 -> 1 は 1、0 -> 0 と 2 -> 2 は 0
//   'c': 2 -> 2 は 1 (そのままだと "abc" が現れる)、0 -> 0 と 1 -> 1 は 0
// 答えは (0, INF, INF) から始めた全体の積の行 0 の最小。
#include "../common.hpp"
constexpr int CM_INF= 1 << 29;
inline const array<int, 9>& char_mat(char c) {
 constexpr int I= CM_INF;
 static const array<int, 9> ma= {1, 0, I, I, 0, I, I, I, 0}, mb= {0, I, I, I, 1, 0, I, I, 0}, mc= {0, I, I, I, 0, I, I, I, 1};
 return c == 'a' ? ma : c == 'b' ? mb : mc;
}
