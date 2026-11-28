// https://onlinejudge.u-aizu.ac.jp/problems/3262
// phi(prod a[l..r]) = prod a[l..r] * prod_{p | prod} (p - 1) / p なので、Mo で区間積と
// 素因数ごとの出現数を動かす。p <= 1e6 < MOD なので逆元が取れる。
#include <cstdio>
#include <vector>
#include "neo/base/engine/query/Mo.hpp"
using namespace std;

namespace {
constexpr int MOD= 998244353;
constexpr int VMAX= 1000000;
}  // namespace

signed main() {
 int n;
 if(scanf("%d", &n) != 1) return 0;
 vector<int> a(n);
 for(auto& x: a) scanf("%d", &x);
 int q;
 scanf("%d", &q);
 Mo mo;
 for(int i= 0; i < q; ++i) {
  int l, r;
  scanf("%d %d", &l, &r), --l;
  mo.query(l, r);
 }
 // 最小素因数。素数のところは自分自身にしておく。
 vector<int> spf(VMAX + 1, 0);
 for(int i= 2; i <= VMAX; ++i)
  if(!spf[i])
   for(int j= i; j <= VMAX; j+= i)
    if(!spf[j]) spf[j]= i;
 for(int i= 2; i <= VMAX; ++i)
  if(!spf[i]) spf[i]= i;
 // 1..VMAX の逆元。MOD % i < i なので前から埋まる。
 vector<int> inv(VMAX + 1);
 inv[1]= 1;
 for(int i= 2; i <= VMAX; ++i) inv[i]= (long long)(MOD - MOD / i) * inv[MOD % i] % MOD;
 // 位置ごとの相異なる素因数を平らに持つ。
 vector<int> fac, off(n + 1);
 for(int i= 0; i < n; ++i) {
  int x= a[i];
  while(x > 1) {
   int p= spf[x];
   fac.push_back(p);
   while(x % p == 0) x/= p;
  }
  off[i + 1]= (int)fac.size();
 }
 vector<int> cnt(VMAX + 1, 0), ans(q);
 long long prod= 1, mult= 1;
 auto add= [&](int i) {
  (prod*= a[i])%= MOD;
  for(int k= off[i]; k < off[i + 1]; ++k) {
   int p= fac[k];
   if(cnt[p]++ == 0) mult= mult * (p - 1) % MOD * inv[p] % MOD;
  }
 };
 auto erase= [&](int i) {
  (prod*= inv[a[i]])%= MOD;
  for(int k= off[i]; k < off[i + 1]; ++k) {
   int p= fac[k];
   if(--cnt[p] == 0) mult= mult * p % MOD * inv[p - 1] % MOD;
  }
 };
 auto out= [&](int id) { ans[id]= (int)(prod * mult % MOD); };
 mo.run(add, erase, out);
 for(int i= 0; i < q; ++i) printf("%d\n", ans[i]);
}
