#pragma once
// 見つからなかった探索の印を次に増加路が見つかるまで残し、幅優先で増加路を探す。
#include "_shared/kuhn/inc_matching.hpp"
using Match = kuhn::IncMatching<1, 0, true>;
#include "kuhn.hpp"
