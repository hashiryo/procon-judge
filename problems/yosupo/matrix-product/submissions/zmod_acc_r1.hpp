#pragma once
// NeoLibrary の ZMod<998244353> で、i-k-j の順に C の行を u64 の和で持ち、8 段ごとに 2^32 の位を畳む。A の行を 1 本ずつ回す (_zmod_mat.hpp の試作)。
#include "_zmod_mat.hpp"
inline vector<u32> run(int N, int M, int P, const vector<u32>& a, const vector<u32>& b) {
 return run_zmod(N, M, P, a, b, [](auto... x) { return mat_mul_acc<1>(x...); });
}
