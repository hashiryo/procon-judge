#pragma once
// 診断用。先読みを抜いた libsais (libsais_nopf と同じ中身) の 1 段目を 5 つの段に分けたもので、run() では 5 つの段をすべて回し、
// 出力の配列の確保と 0 埋めだけを構築子に出す。libsais_nopf との差が、出力の配列の確保とページフォールトの分。段の分け方と仕組みは
// _diag_split_libsais.hpp にある。調べ終えたら消す。
#define DIAG_PHASE 0
#include "_diag_split_libsais.hpp"
