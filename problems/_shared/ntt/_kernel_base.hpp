#pragma once
// self/ntt の核の実験で共通の小さな定義。核のヘッダが 2 つ以上あっても二重定義にならないようにここへ置く。
// 単独の NTT の核を既定にしてよい素数か。2^20 までの変換を約束する (工具の大きさは後で決める)。
#include "neo/base/core/nt/is_prime.hpp"

template <unsigned P> constexpr bool ntt_friendly= P > 2 && is_prime(P) && (P - 1) % (1u << 20) == 0;
