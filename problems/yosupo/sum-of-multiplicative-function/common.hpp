#pragma once
// ライブラリを使う提出が共有するもの。法は 469762049 (素数)。
#include "pj.hpp"
#include "mylib/algebra/ModInt.hpp"

constexpr unsigned MF_MOD = 469762049;
using Mint = ModInt<MF_MOD>;
