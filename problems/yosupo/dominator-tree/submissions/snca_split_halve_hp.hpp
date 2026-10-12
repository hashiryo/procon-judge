#pragma once
// snca_split_pf_hp の eval を、道を 1 回登りながら親を祖父に付け替える経路半減にし、スタックに積んで上から直す 2 回の走査をやめる。核は _dom.hpp。
#include "_dom.hpp"
using Solver= dom::Solver<6>;
