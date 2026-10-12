#pragma once
// tsin_crec_hp で、2-core の次数 2 の頂点の鎖を DFS の中でその場で 1 本の辺に縮め (鎖の頂点はどれも 1 頂点の成分)、核の頂点 (次数 3 以上) だけを Tsin の DFS に載せる。核は _t3.hpp。
#include "_t3.hpp"
using Solver= t3::Solver<2>;
