// 愚直解。頂点が 10 個までの入力で、s を含み t を含まない頂点集合を全部試して最小カットを全部数え上げ、辺ごとに、どれかの
// 最小カットに入るか (始点が s の側、終点が t の側) と、すべての最小カットに入るかを出す。
#include <cstdio>
#include <vector>
int main() {
 int n, m, s, t;
 if(scanf("%d %d %d %d", &n, &m, &s, &t) != 4 || n > 12) return 1;
 --s, --t;
 std::vector<int> u(m), v(m);
 std::vector<long long> c(m);
 for(int i= 0; i < m; ++i) {
  if(scanf("%d %d %lld", &u[i], &v[i], &c[i]) != 3) return 1;
  --u[i], --v[i];
 }
 auto cut= [&](int x) {
  long long r= 0;
  for(int i= 0; i < m; ++i)
   if((x >> u[i] & 1) && !(x >> v[i] & 1)) r+= c[i];
  return r;
 };
 long long best= -1;
 for(int x= 0; x < (1 << n); ++x)
  if((x >> s & 1) && !(x >> t & 1) && (best < 0 || cut(x) < best)) best= cut(x);
 std::vector<int> some(m, 0), every(m, 1);
 for(int x= 0; x < (1 << n); ++x) {
  if(!(x >> s & 1) || (x >> t & 1) || cut(x) != best) continue;
  for(int i= 0; i < m; ++i) {
   const bool in= (x >> u[i] & 1) && !(x >> v[i] & 1);
   some[i]|= in, every[i]&= in;
  }
 }
 for(int i= 0; i < m; ++i) printf("%d %d\n", some[i], every[i]);
}
