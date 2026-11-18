#pragma once
// 見つからなかった探索の印を次に増加路が見つかるまで残し、lookahead の位置を探索をまたいで覚える Kuhn 法。
#include "_shared/kuhn/inc_matching.hpp"
using Match = kuhn::IncMatching<1, 2, false>;
#include "common.hpp"
