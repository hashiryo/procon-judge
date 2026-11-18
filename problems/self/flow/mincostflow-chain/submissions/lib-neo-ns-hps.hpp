#pragma once
// 診断用。2 MB 以上の確保を huge page に置き、確保の先頭を取るたびに 2 KB + 64 byte の倍数だけずらす (_shared/pages/plain_new.hpp
// の PAGES_HP_STAGGER)。ネットワーク単体法の弧ごとの配列 (端、容量、流量、費用) とポテンシャルが 2 MB の区画に載る。同じ束の
// lib-neo-ns と比べる。比べ終えたら消す。
#define PAGES_HP_STAGGER
#include "_shared/pages/plain_new.hpp"
#include "lib-neo-ns.hpp"
