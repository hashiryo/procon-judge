#pragma once
// ライブラリを使う提出が共有するもの。
//
// 添字を値そのものにして、その値が集合にあるかを 0/1 で持つ。xor した順で
// 見たときの最初の 1 を探せば答えになるので、載せるのは区間和のモノイド。
#include "pj.hpp"

struct CountSum {
  using T = int;
  static T ti() { return 0; }
  static T op(const T &l, const T &r) { return l + r; }
};
