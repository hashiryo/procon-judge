#pragma once
// run() で 4 MB の領域を 1 で埋めるだけの診断用の提出 (MODE 2、中身は _diag_touch.hpp)。
// 構築で触っておいた領域を埋め直す。
// run の列挙の実装を比べた記録は algo-notes の notes/runs.md にある。
#include "pj.hpp"
#include "_diag_touch.hpp"

using Solver = DiagTouchSolver<2>;
