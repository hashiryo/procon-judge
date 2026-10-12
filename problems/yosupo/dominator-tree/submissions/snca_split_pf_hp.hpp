#pragma once
// snca_split_hp で、sdom の走査のときに 8 個先の頂点の逆向きの辺の始点の行 (森の親と最小の値) を先読みする。核は _dom.hpp。
#include "_dom.hpp"
using Solver= dom::Solver<5>;
