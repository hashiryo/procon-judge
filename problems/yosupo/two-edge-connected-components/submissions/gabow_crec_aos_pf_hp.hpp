#pragma once
// gabow_crec_aos_d8pf_hp から、CSR を組むときの番兵を 8 つに回す手を外し、先読みだけを残す。核は _tecc2.hpp。
#include "_tecc2.hpp"
using Solver= tecc2::Solver<true, true, false>;
