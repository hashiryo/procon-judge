#pragma once
// 添字の列を std::sort で (値, 添字) の順に並べる。比べるたびに a を添字で引く。
#include <algorithm>
#include <numeric>
#include <vector>
inline std::vector<unsigned> run(const std::vector<unsigned>& a) {
 std::vector<unsigned> p(a.size());
 std::iota(p.begin(), p.end(), 0u);
 std::sort(p.begin(), p.end(), [&](unsigned i, unsigned j) { return a[i] < a[j] || (a[i] == a[j] && i < j); });
 return p;
}
