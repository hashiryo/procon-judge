#pragma once
// lt と同じ経路圧縮だけの Lengauer–Tarjan を、huge page を頼む 1 本の作業領域と、届く頂点の番号の上で組み直した逆向きの CSR で回す。 核は _dom.hpp。
#include "_dom.hpp"
using Solver= dom::Solver<0>;
