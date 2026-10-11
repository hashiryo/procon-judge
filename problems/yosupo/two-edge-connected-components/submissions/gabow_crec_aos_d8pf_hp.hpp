#pragma once
// gabow_crec_aos_d8_hp で、次数と XOR を数える走査と CSR を組む走査に、16 本先と 8 本先の辺の端の先読みを足す。核は _tecc2.hpp。
#include "_tecc2.hpp"
using Solver= tecc2::Solver<true, true, true>;
