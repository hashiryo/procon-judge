#pragma once
// 診断用。先読みを抜いた libsais (libsais_nopf と同じ中身) の 1 段目を 5 つの段に分けたもので、run() では 2 番目の段だけを
// 回す。段の分け方と仕組みは _diag_split_libsais.hpp にある。調べ終えたら消す。
#define DIAG_PHASE 2
#include "_diag_split_libsais.hpp"
