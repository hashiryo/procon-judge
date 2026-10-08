#pragma once
// 添字の列を std::stable_sort で値の順に並べる。等しい値は元の順 (添字の小さい順) に残る。期待出力もこれで作る。
#include <algorithm>
#include <numeric>
#include <vector>
inline std::vector<unsigned> run(const std::vector<unsigned>& a) {
 std::vector<unsigned> p(a.size());
 std::iota(p.begin(), p.end(), 0u);
 std::stable_sort(p.begin(), p.end(), [&](unsigned i, unsigned j) { return a[i] < a[j]; });
 return p;
}
