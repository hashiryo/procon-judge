#pragma once
// 書き比べ: 速い核の手直しの試し (_shared/sps/ranked_xor_v3.hpp、NeoLibrary eaf3ecf の核から) を、詰めて持つ形 (RX3_CMP) と次の列の先読み (RX3_PF) の両方を入れて呼ぶ。
// 中身は _v3.hpp。比べ終えたら消す。
#define RX3_CMP 1
#define RX3_PF 1
#include "_v3.hpp"
