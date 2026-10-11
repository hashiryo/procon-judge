#pragma once
// 要を 16 本ずつまとめて掃き出す GF(2) の核 (_shared/linalg/gf2_elim.hpp) で行列式を求める (正則なら 1)。
#include "../common.hpp"
#include "_shared/linalg/gf2_elim.hpp"
inline int run(int n, const vector<string>& a) {
 gf2_elim::Elim e(n, ((n + 63) / 64 + 3) & ~3);
 for(int i= 0; i < n; ++i) gf2_elim::pack_row(a[i].data(), n, e.row(i));
 return e.run(n, true) ? 1 : 0;
}
