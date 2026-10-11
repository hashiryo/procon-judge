#pragma once
// 要を 16 本ずつまとめて掃き出す GF(2) の核 (_shared/linalg/gf2_elim.hpp) で逆行列を求める。
// [A | I] を前進消去してから RREF に直すと、要の行 t (要の列は t) の右半分が A^{-1} の行 t になる。
#include "../common.hpp"
#include "_shared/linalg/gf2_elim.hpp"
inline vector<string> run(int n, const vector<string>& a) {
 gf2_elim::Elim e(n, ((2 * n + 63) / 64 + 3) & ~3);
 for(int i= 0; i < n; ++i) {
  gf2_elim::pack_row(a[i].data(), n, e.row(i));
  e.row(i)[(n + i) >> 6]|= u64(1) << ((n + i) & 63);
 }
 if(!e.run(n, true)) return {};
 e.back();
 vector<string> res(n, string(n, '0'));
 for(int t= 0; t < n; ++t) gf2_elim::unpack_bits(e.urow(t), n, n, res[t].data());
 return res;
}
