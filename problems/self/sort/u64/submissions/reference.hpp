#pragma once
// std::sort。期待出力もこれで作る。
#include <algorithm>
#include <vector>
inline void run(std::vector<unsigned long long>& a) { std::sort(a.begin(), a.end()); }
