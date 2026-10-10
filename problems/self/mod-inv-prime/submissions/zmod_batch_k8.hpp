#pragma once
// NeoLibrary の ZMod<998244353> の値を、積の鎖を 8 本に分けた Montgomery の方法でまとめて逆元にする (_batch_inv.hpp の試作)。
#include "_batch_inv.hpp"
inline vector<u32> run(u32, const vector<u32>& qs) {
 return run_batch(qs, [](const auto& a) { return batch_inv<8>(a); });
}
