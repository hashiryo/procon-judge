#pragma once
// _shared/kuhn の核で dyn と同じ探し方 (幅優先、探すたびに印を消す) をするもの。核の口を通す分の重さを見る。
#include "_shared/kuhn/inc_matching.hpp"
using Match = kuhn::IncMatching<0, 0, true>;
#include "kuhn.hpp"
