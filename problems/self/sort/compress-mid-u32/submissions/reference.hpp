#pragma once
// Library の compress と同じ手順。a を写して std::sort と std::unique で xs を作り、各要素を std::lower_bound で引いて順位に
// 置き換える。期待出力もこれで作る。
#include <algorithm>
#include <vector>
inline std::vector<unsigned> run(std::vector<unsigned>& a) {
 std::vector<unsigned> xs(a);
 std::sort(xs.begin(), xs.end());
 xs.erase(std::unique(xs.begin(), xs.end()), xs.end());
 for(auto& x: a) x= unsigned(std::lower_bound(xs.begin(), xs.end(), x) - xs.begin());
 return xs;
}
