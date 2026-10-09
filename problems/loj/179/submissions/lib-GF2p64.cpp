// https://loj.ac/p/179
// 32 bit の nimber の積を GF2p64 で計算する。2^32 未満の nimber は部分体をなすので、from_nimber で移して掛け、
// to_nimber で戻せば 2^32 未満に収まる。次の x は lastans を整数として足すので、毎回 nimber の表現に戻す
#include <iostream>
#include "neo/algebra/GF2p64.hpp"
using namespace std;
using u32= unsigned;
u32 SA, SB, SC;
u32 rng() {
 SA^= SA << 16;
 SA^= SA >> 5;
 SA^= SA << 1;
 const u32 t= SA;
 SA= SB;
 SB= SC;
 SC^= t ^ SA;
 return SC;
}
signed main() {
 int T;
 cin >> T >> SA >> SB >> SC;
 u32 lastans= 0;
 while(T--) {
  const u32 x= rng() + lastans, y= rng();
  lastans= u32((GF2p64::from_nimber(x) * GF2p64::from_nimber(y)).to_nimber());
 }
 cout << lastans << '\n';
 return 0;
}
