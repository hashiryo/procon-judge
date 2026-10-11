#pragma once
// 剥がしたあとの 2-core で、次数 2 の頂点が続く鎖を 16 本ずつ交互に歩いて 1 本の辺に縮め、残った核 (次数 3 以上の頂点と、次数 2 の頂点のうち 32 個に 1 個ほど) だけを記録の形の Hopcroft–Tarjan で分ける。橋の鎖は辺ごとの成分に、輪の鎖は鎖だけの成分に開く。核は _bcc3.hpp。
#include "_bcc3.hpp"
using Solver= bcc3::Solver<5, 16>;
