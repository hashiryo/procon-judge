#pragma once
// SEMI-NCA。準支配点は Lengauer–Tarjan と同じく経路圧縮の eval で求め、直接支配点は木の親から sdom より上の直接支配点をたどって決める。 核は _dom.hpp。
#include "_dom.hpp"
using Solver= dom::Solver<1>;
