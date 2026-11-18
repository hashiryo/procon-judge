#pragma once
// 見つからなかった探索の印を、増加路が見つかっても消さない (見つかった探索で訪ねた頂点の印だけ消す) Kuhn 法。lookahead の位置を探索をまたいで覚える。
#include "_shared/kuhn/inc_matching.hpp"
using Match = kuhn::IncMatching<2, 2, false>;
#include "common.hpp"
