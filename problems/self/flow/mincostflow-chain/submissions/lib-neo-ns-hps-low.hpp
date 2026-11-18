#pragma once
// 診断用。lib-neo-ns-hps の閾値を 512 KB に下げたもの。30 万頂点の木の配列 (親、木の辺、thread、部分木の大きさなど。int で
// 1.2 MB) も、それぞれ 2 MB の区画に載る。同じ束の lib-neo-ns、lib-neo-ns-hps と比べる。比べ終えたら消す。
#define PAGES_HP_STAGGER
#define PAGES_HP_MIN (std::size_t(1) << 19)
#include "_shared/pages/plain_new.hpp"
#include "lib-neo-ns.hpp"
