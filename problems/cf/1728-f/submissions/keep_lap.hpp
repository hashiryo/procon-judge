#pragma once
// keep_la の lookahead の位置を探索をまたいで覚え、続きから見る Kuhn 法 (Pothen-Fan の lookahead と同じ)。
#include "_shared/kuhn/inc_matching.hpp"
using Match = kuhn::IncMatching<1, 2, false>;
#include "common.hpp"
