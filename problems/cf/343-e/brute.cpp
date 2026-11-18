// 愚直解。駅が 8 個までの入力で、全部の組の最小カットを頂点集合を全部試して求め、順列を全部試して和の最大を出す (Gomory-Hu
// 木を使わない)。和が最大の順列のうち、辞書順で最初のものを出す。
#include <algorithm>
#include <cstdio>
#include <numeric>
#include <vector>
int main() {
 int n, m;
 if(scanf("%d %d", &n, &m) != 2) return 1;
 std::vector<int> a(m), b(m), c(m);
 for(int i= 0; i < m; ++i) {
  if(scanf("%d %d %d", &a[i], &b[i], &c[i]) != 3) return 1;
  --a[i], --b[i];
 }
 if(n > 10) return 1;
 const int full= 1 << n;
 std::vector<long long> cut(full, 0);
 for(int x= 0; x < full; ++x)
  for(int i= 0; i < m; ++i)
   if((x >> a[i] & 1) != (x >> b[i] & 1)) cut[x]+= c[i];
 std::vector<std::vector<long long>> lam(n, std::vector<long long>(n, -1));
 for(int s= 0; s < n; ++s)
  for(int t= 0; t < n; ++t)
   if(s != t)
    for(int x= 0; x < full; ++x)
     if((x >> s & 1) && !(x >> t & 1) && (lam[s][t] < 0 || cut[x] < lam[s][t])) lam[s][t]= cut[x];
 std::vector<int> p(n), best;
 std::iota(p.begin(), p.end(), 0);
 long long mx= -1;
 do {
  long long sum= 0;
  for(int i= 0; i + 1 < n; ++i) sum+= lam[p[i]][p[i + 1]];
  if(sum > mx) mx= sum, best= p;
 } while(std::next_permutation(p.begin(), p.end()));
 printf("%lld\n", mx);
 for(int i= 0; i < n; ++i) printf("%d%c", best[i] + 1, i + 1 < n ? ' ' : '\n');
}
