#pragma once
// snca_split_hp の eval を、道を 1 回登りながら親を祖父に付け替える経路半減にする (snca_split_halve_hp から sdom の走査の先読みを外したもの)。核は _dom.hpp。
#include "_dom.hpp"
using Solver= dom::Solver<7>;
