#pragma once
// tarjan_crec_aos_hp で、CSR を組むときに捨てる辺の行き先を 8 つの番兵に回し、番兵の書き込み位置を動かさない (同じ番地の読み書きが辺ごとに直列につながらない)。核は _bcc2.hpp。
#include "_bcc2.hpp"
using Solver= bcc2::Solver<true, false, true>;
