#pragma once
// 書き比べ: 速い核の手直しの試し (_shared/sps/ranked_xor_v3.hpp、NeoLibrary eaf3ecf の核から) を、列ごとの段で、次の列で読む塊を先に読む形 (RX3_PF) だけを入れて呼ぶ。
// 中身は _v3.hpp。比べ終えたら消す。
#define RX3_CMP 0
#define RX3_PF 1
#include "_v3.hpp"
