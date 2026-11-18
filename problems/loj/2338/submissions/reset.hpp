#pragma once
// 教科書の Kuhn 法。探すたびに訪ねた印を消し、空いた右の隣を先に探さない深さ優先。
#include "_shared/kuhn/inc_matching.hpp"
using Match = kuhn::IncMatching<0, 0, false>;
#include "common.hpp"
