#pragma once
// gabow_chain_hp で、核の CSR を組むときの番兵を 8 つに回す (同じ番地の読み書きが直列につながらない)。核は _tecc3.hpp。
#include "_tecc3.hpp"
using Solver= tecc3::Solver<5, 16, true>;
