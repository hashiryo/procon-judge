#pragma once
// keep_lap の印を、増加路が見つかっても消さない形 (見つかった探索で訪ねた頂点の印だけ消す)。
#include "_shared/kuhn/inc_matching.hpp"
using Match = kuhn::IncMatching<2, 2, false>;
#include "common.hpp"
