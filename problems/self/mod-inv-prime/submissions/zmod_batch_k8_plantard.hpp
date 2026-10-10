#pragma once
// zmod_batch_k8 の掛け算を、値を Plantard の表現に直さない P(x, y) = -x y 2^{-64} mod M にした版 (_batch_inv.hpp の試作)。
#include "_batch_inv.hpp"
inline vector<u32> run(u32, const vector<u32>& qs) {
 return run_batch(qs, [](const auto& a) { return batch_inv_plantard<8>(a); });
}
