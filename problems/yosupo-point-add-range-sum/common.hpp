#pragma once
// 提出とハーネスが共通で使うもの。
// bits/stdc++.h は Apple clang に無いので、必要なものを名指しで include する。
#include <array>
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <vector>

using namespace std;

// scanf の %lld と揃えたいので int64_t ではなく long long を使う。
using i64 = long long;
using u64 = unsigned long long;
using u32 = unsigned int;
