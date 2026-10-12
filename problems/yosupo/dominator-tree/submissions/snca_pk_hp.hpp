#pragma once
// snca_hp で、逆向きの CSR を組むときに辺の端の番号を 1 回だけ引き (1 回目の走査で引いた番号の組を並べて残す)、eval の森の親と道の上の sdom の最小の値を 1 つの配列に並べる。核は _dom.hpp。
#include "_dom.hpp"
using Solver= dom::Solver<3>;
