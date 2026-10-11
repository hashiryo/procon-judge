#pragma once
// gabow_crec_aos_hp で、CSR を組むときに捨てる辺の行き先を 8 つの番兵に回し、番兵の書き込み位置を動かさない (同じ番地の読み書きが辺ごとに直列につながらない)。核は _tecc2.hpp。
#include "_tecc2.hpp"
using Solver= tecc2::Solver<true, false, true>;
