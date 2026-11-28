#pragma once
#include <algorithm>
#include <vector>
// 行 r の有効な列が [lo(r), hi(r)) のときの、各行の最小の列を分割統治で求める。value の評価は O(h log w) 回。
// lo と hi は広義単調増加で、どの行も lo(r) < hi(r)。行の最小の列が行について単調であること (Monge など)。
// 有効でないセルを評価しないので、値の関数の中で範囲を判定して T の最大値を返す形より速い。
// 有効な列が階段になる DP の各層 (dp[i] = min_{j < i} dp[j] + w(j, i) など) で使う。
template <class T, class F, class Lo, class Hi> std::vector<int> monotone_minima_bounded(int h, const F& value, const Lo& lo, const Hi& hi) {
 std::vector<int> best(h);
 if(h == 0) return best;
 auto rec= [&](auto& rec, int h1, int h2, int w1, int w2) -> void {
  if(h1 == h2) return;
  const int hm= (h1 + h2) / 2;
  const int wl= std::max(w1, lo(hm)), wr= std::min(w2, hi(hm));
  int bj= wl;
  T bv= value(hm, wl);
  for(int j= wl + 1; j < wr; ++j) {
   const T v= value(hm, j);
   if(v < bv) bv= v, bj= j;
  }
  best[hm]= bj;
  rec(rec, h1, hm, w1, bj + 1);
  rec(rec, hm + 1, h2, bj, w2);
 };
 rec(rec, 0, h, lo(0), hi(h - 1));
 return best;
}
// 上の関数の、各行の最小の値だけを返す形。呼ぶ側が添字から値を引き直さないので、DP の各層のように
// 最小の値がそのまま次の層に要るときはこちらが速い。
template <class T, class F, class Lo, class Hi> std::vector<T> monotone_minima_bounded_values(int h, const F& value, const Lo& lo, const Hi& hi) {
 std::vector<T> best(h);
 if(h == 0) return best;
 auto rec= [&](auto& rec, int h1, int h2, int w1, int w2) -> void {
  if(h1 == h2) return;
  const int hm= (h1 + h2) / 2;
  const int wl= std::max(w1, lo(hm)), wr= std::min(w2, hi(hm));
  int bj= wl;
  T bv= value(hm, wl);
  for(int j= wl + 1; j < wr; ++j) {
   const T v= value(hm, j);
   if(v < bv) bv= v, bj= j;
  }
  best[hm]= bv;
  rec(rec, h1, hm, w1, bj + 1);
  rec(rec, hm + 1, h2, bj, w2);
 };
 rec(rec, 0, h, lo(0), hi(h - 1));
 return best;
}
