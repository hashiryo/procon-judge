#pragma once
// Library の MP_Mo32 をそのまま使う (32 bit の Montgomery。奇数 mod < 2^30 (ModInt が使う範囲))。
// mylib/internal/Remainder.hpp が変わると測り直される。
#include "../../_shared/modulo-test/_common.hpp"
#include "mylib/internal/Remainder.hpp"
using MP= math_internal::MP_Mo32;
