#pragma once
// Library の MP_Mo64 をそのまま使う (64 bit の Montgomery。奇数 mod < 2^62)。
// mylib/internal/Remainder.hpp が変わると測り直される。
#include "../../_shared/modulo-test/_common.hpp"
#include "mylib/internal/Remainder.hpp"
using MP= math_internal::MP_Mo64;
