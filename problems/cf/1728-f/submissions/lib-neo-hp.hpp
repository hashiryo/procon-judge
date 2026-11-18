#pragma once
// lib-neo の型の写しで、2 MB 以上の配列を huge page に置き先頭をずらす形 (_shared/kuhn/neo_hp.hpp) の試し。
#include "_shared/kuhn/neo_hp.hpp"
using Match = kuhn_hp::IncrementalBipartiteMatchingHP;
#include "common.hpp"
