#pragma once
// 掛ける定数。コンパイル時に分かっているので、提出はこれを前提にした形 (定数倍用の
// 表を焼くなど) を取ってよい。
//
// 値は適当に選んだ密なもの (立っている bit が 32 本前後) にしてある。疎な定数だと
// shift と XOR だけの実装が一方的に有利になって、比べたいものが比べられないため。
#include "_shared/gf2-64/_common.hpp"
constexpr u64 MUL_CONST= 0x9e3779b97f4a7c15ull;
