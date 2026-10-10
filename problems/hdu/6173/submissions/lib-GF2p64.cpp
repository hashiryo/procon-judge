// https://acm.hdu.edu.cn/showproblem.php?pid=6173
// 2 次元の Ruler game なので、表の硬貨 (x, y) の SG は lowbit(x) ⊗ lowbit(y) (nim 積)。矩形の和集合の上でその xor をとる。
// x で掃き、y の被覆をセグメント木で持つ。帯 [x, x') の寄与は (lowbit の xor の x 方向の区間和) ⊗ (覆われた y の lowbit の xor) で、
// xor は nimber でも GF2p64 でも同じなので、積をとるときだけ from_nimber で移す。勝ち負けは和が 0 かどうかだけで決まる
#include <cstdio>
#include <vector>
#include <algorithm>
#include "neo/algebra/GF2p64.hpp"
using namespace std;
using u64= unsigned long long;
static char ibuf[1 << 25];
size_t ipos, ilen;
u64 read_u64() {  // 入力は 12 MB ほどになるので fread で読む
 auto get= [&]() { return ipos < ilen || (ipos= 0, (ilen= fread(ibuf, 1, sizeof(ibuf), stdin)) != 0) ? ibuf[ipos++] : '\0'; };
 char c= get();
 while(c < '0' || c > '9') c= get();
 u64 x= 0;
 for(; c >= '0' && c <= '9'; c= get()) x= x * 10 + (c - '0');
 return x;
}
// 1 以上 n 以下の lowbit の xor。lowbit が 2^t の数は floor(n/2^t) - floor(n/2^(t+1)) 個
u64 lowbit_xor(u64 n) {
 u64 r= 0;
 for(int t= 0; n >> t; ++t)
  if(((n >> t) - (n >> (t + 1))) & 1) r|= u64(1) << t;
 return r;
}
signed main() {
 int T= read_u64();
 while(T--) {
  read_u64();  // N
  const int M= read_u64();
  vector<u64> ys;
  struct Event {
   u64 x, lo, hi;  // y の半開区間 [lo, hi)
   int d;
  };
  vector<Event> ev;
  for(int i= 0; i < M; ++i) {
   const u64 x1= read_u64(), y1= read_u64(), x2= read_u64(), y2= read_u64();
   ev.push_back({x1, y1, y2 + 1, 1}), ev.push_back({x2 + 1, y1, y2 + 1, -1});
   ys.push_back(y1), ys.push_back(y2 + 1);
  }
  sort(ys.begin(), ys.end()), ys.erase(unique(ys.begin(), ys.end()), ys.end());
  sort(ev.begin(), ev.end(), [](const Event& a, const Event& b) { return a.x < b.x; });
  // 葉 i は [ys[i], ys[i+1])。cnt は覆う矩形の数、val は覆われた y の lowbit の xor、full は区間全体の lowbit の xor
  const int L= ys.size() - 1;
  vector<int> cnt(4 * L);
  vector<u64> val(4 * L), full(4 * L);
  auto build= [&](auto&& self, int k, int l, int r) -> void {
   full[k]= lowbit_xor(ys[r] - 1) ^ lowbit_xor(ys[l] - 1);
   if(r - l > 1) self(self, 2 * k, l, (l + r) / 2), self(self, 2 * k + 1, (l + r) / 2, r);
  };
  build(build, 1, 0, L);
  auto update= [&](auto&& self, int k, int l, int r, int a, int b, int d) -> void {
   if(b <= l || r <= a) return;
   if(a <= l && r <= b) cnt[k]+= d;
   else {
    const int m= (l + r) / 2;
    self(self, 2 * k, l, m, a, b, d), self(self, 2 * k + 1, m, r, a, b, d);
   }
   val[k]= cnt[k] ? full[k] : r - l == 1 ? 0 : val[2 * k] ^ val[2 * k + 1];
  };
  GF2p64 total;
  for(size_t i= 0; i < ev.size();) {
   const u64 x= ev[i].x;
   for(; i < ev.size() && ev[i].x == x; ++i) {
    const int a= lower_bound(ys.begin(), ys.end(), ev[i].lo) - ys.begin(), b= lower_bound(ys.begin(), ys.end(), ev[i].hi) - ys.begin();
    update(update, 1, 0, L, a, b, ev[i].d);
   }
   if(i < ev.size() && val[1]) total+= GF2p64::from_nimber(lowbit_xor(ev[i].x - 1) ^ lowbit_xor(x - 1)) * GF2p64::from_nimber(val[1]);
  }
  puts(total ? "Yong Chol" : "Brother");
 }
 return 0;
}
