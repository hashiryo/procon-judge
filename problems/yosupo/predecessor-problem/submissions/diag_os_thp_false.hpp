#pragma once
// 診断用。NeoLibrary の OrderedSet<false> を、葉と節点の配列の置き方を変えずに呼ぶ (lib-neo-ordered-set と同じ機械語)。
// 同じ束の中で時刻をずらして測り、diag_os_hp_false と diag_os_4k_false と比べる。
#include "_neo_os.hpp"
using Solver = NeoOrderedSetSolver<false>;
