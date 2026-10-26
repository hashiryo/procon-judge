#pragma once
// NeoLibrary の RangeXorBasis (neo/linalg/XorBasis.hpp)。この問題で書き比べた snap16_red と同じく、接頭辞の基底の写しを
// 16 個おきに持ち、問い合わせでは写しの基底と残りの元を min の形で消して判定する。
#include "../common.hpp"
#include "neo/linalg/XorBasis.hpp"
struct Solver {
 RangeXorBasis<u64> rb;
 explicit Solver(const vector<u64>& a): rb(a) {}
 bool query(int l, int r, u64 x) const { return rb.contains(l, r, x); }
};
