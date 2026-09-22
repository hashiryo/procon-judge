#pragma once
// Library の MP_D2B1_2 をそのまま使う (div2by1。mod < 2^64)。
// mylib/internal/Remainder.hpp が変わると測り直される。
#include "../../_shared/modulo-test/_common.hpp"
#include "mylib/internal/Remainder.hpp"
using MP= math_internal::MP_D2B1_2;
