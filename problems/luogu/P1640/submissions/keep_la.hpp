#pragma once
// keep に、左の頂点に入るたびに空いた右の隣を先に全部見る手 (lookahead) を足した Kuhn 法。
#include "_shared/kuhn/inc_matching.hpp"
using Match = kuhn::IncMatching<1, 1, false>;
#include "kuhn.hpp"
