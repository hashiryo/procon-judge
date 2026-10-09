#pragma once
// Library の MP_Na をそのまま使う (u64 の % を使う素朴な剰余。mod < 2^32)。
// mylib/internal/Remainder.hpp が変わると測り直される。
#include "_shared/modulo-test/_common.hpp"
#include "mylib/internal/Remainder.hpp"
using MP= math_internal::MP_Na;
