#pragma once
// tarjan_crec_aos_d8_hp で、次数と XOR を数える走査と CSR を組む走査に、16 本先と 8 本先の辺の端の先読みを足す。核は _bcc2.hpp。
#include "_bcc2.hpp"
using Solver= bcc2::Solver<true, true, true>;
