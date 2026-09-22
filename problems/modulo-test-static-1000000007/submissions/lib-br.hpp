#pragma once
// Library の MP_Br をそのまま使う (Barrett (2^84 / mod)。2^20 < mod <= 2^41)。
// mylib/internal/Remainder.hpp が変わると測り直される。
#include "../../_shared/modulo-test/_common.hpp"
#include "mylib/internal/Remainder.hpp"
using MP= math_internal::MP_Br;
