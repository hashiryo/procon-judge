#pragma once
// 診断用。NeoLibrary の suffix_array (lib-neo と同じ中身) の 1 段目を 5 つの段に分けたもので、run() では 3 番目の段だけを回す。
// 段の分け方と仕組みは _diag_split_neo.hpp にある。調べ終えたら消す。
#define DIAG_PHASE 3
#include "_diag_split_neo.hpp"
