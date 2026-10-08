#pragma once
// std::stable_sort (マージソート)。作業用の配列を確保する比較ソートの基準。
#include <algorithm>
#include <vector>
inline void run(std::vector<unsigned>& a) { std::stable_sort(a.begin(), a.end()); }
