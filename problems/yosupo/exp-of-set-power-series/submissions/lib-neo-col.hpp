#pragma once
// 書き比べ: 速い核の手直しの試し (_shared/sps/ranked_xor_v2.hpp) を手直しを両方入れて使い、上の段を列ごとに済ませる形
// (_shared/sps/ranked_xor_col.hpp) で呼ぶ。lib-neo-v2 との差が列ごとの形の分。中身は _v2.hpp。比べ終えたら消す。
#define RX2_COL 1
#include "_v2.hpp"
