#pragma once
// 書き比べ: 層 8 以上の下の組を 4 個にし、上の組の部分集合ごとの隣接行列を、出入りした組の行と列だけ差分で直す形 (_layer_i.hpp)。
// lib-neo (下の組は w/2 個で、TH ごとに作り直す) と比べる。比べ終えたら消す。
#define HAF_SL 4
#include "_layer_i.hpp"
