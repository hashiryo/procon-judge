#pragma once
// 見つからなかった探索の印を次に増加路が見つかるまで残す Kuhn 法。空いた右の隣を先に探さない深さ優先。
#include "_shared/kuhn/inc_matching.hpp"
using Match = kuhn::IncMatching<1, 0, false>;
#include "kuhn.hpp"
