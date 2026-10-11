#pragma once
// 診断用。tsin_crec_hp の、CSR と記録を組むところまでを測り、残りは最初の comp() で回し直す。核は _t3.hpp。
#include "_t3.hpp"
using Solver= t3::Solver<true, true, 2>;
