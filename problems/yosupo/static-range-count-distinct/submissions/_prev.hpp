#pragma once
// static-range-count-distinct の wm4、wm_sep、pst が共有する前処理。値を順位に詰め、各位置について同じ値が前に
// 出た位置 + 1 (無ければ 0) の列を作る。[l, r) の異なる値の数は、この列の [l, r) のうち l + 1 未満のものの数。
#include "pj.hpp"
#include "_shared/wavelet/wm.hpp"
inline vector<unsigned> prev_plus_one(const vector<int>& a) {
 const int n= a.size();
 const auto c= wavelet::compress(a);
 vector<int> last(c.vals.size(), -1);
 vector<unsigned> p(n);
 for(int i= 0; i < n; ++i) p[i]= last[c.id[i]] + 1, last[c.id[i]]= i;
 return p;
}
