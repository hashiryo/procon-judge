#pragma once
#include "_shared/modulo-test/_common.hpp"
// a, b はどちらも奇数。Library の binary_gcd と同じ形の Stein のループで、差の末尾の 0 を数えてから大小を選ぶ。
// 鎖は引き算、tzcnt、シフトの 3 段になる。
inline u64 stein_odd(u64 a, u64 b) {
 while(a != b) {
  u64 d= a - b;
  int s= __builtin_ctzll(d);
  bool f= a > b;
  b= f ? b : a, a= (f ? d : -d) >> s;
 }
 return a;
}
// 大きいほうを小さいほうで 1 回割ってから Stein に入る。T は割るかを決める桁数の差 (T < 0 ならいつも割る)。
// Stein は大きさが違う組で 1 段に 2 bit ほどしか縮まないので、差が大きいときは割り算 1 回のほうが速い。
template <int T> inline u64 gcd_div1(u64 a, u64 b) {
 u64 mx= std::max(a, b), mn= std::min(a, b);
 if(mn == 0) return mx;
 if(T < 0 || __builtin_clzll(mn) - __builtin_clzll(mx) > T) {
  mx%= mn;
  if(mx == 0) return mn;
 }
 int z= __builtin_ctzll(mx | mn);
 return stein_odd(mx >> __builtin_ctzll(mx), mn >> __builtin_ctzll(mn)) << z;
}
