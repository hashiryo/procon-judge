#pragma once
// ライブラリを使う提出が共有するもの。費用関数はどちらも同じなので、ここに
// 置いて解き方だけを変える。
#include <cstdlib>
#include "pj.hpp"

// 1 項の重み。2 乗を返す。
inline i64 pen(i64 v) { return v * v; }
