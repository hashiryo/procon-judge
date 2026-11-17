// cf-547-d の愚直解。点の塗り方を全部試し、どの縦の線と横の線でも r と b の数の差が 1 以下になるものを 1 つ出す。Euler 分割を
// 使わないので、提出とは別の考え方になる。n <= 12 の入力でだけ使う (gen.py の seed 1000 以上)。
#include <cstdio>
#include <cstdlib>
#include <map>
#include <vector>
int main() {
 int n;
 if(scanf("%d", &n) != 1) return 1;
 std::vector<int> x(n), y(n);
 for(int i= 0; i < n; ++i)
  if(scanf("%d %d", &x[i], &y[i]) != 2) return 1;
 for(int s= 0; s < (1 << n); ++s) {
  std::map<int, int> dx, dy;
  for(int i= 0; i < n; ++i) {
   const int d= s >> i & 1 ? 1 : -1;
   dx[x[i]]+= d, dy[y[i]]+= d;
  }
  bool ok= true;
  for(auto& [k, v]: dx) ok&= abs(v) <= 1;
  for(auto& [k, v]: dy) ok&= abs(v) <= 1;
  if(ok) {
   for(int i= 0; i < n; ++i) putchar(s >> i & 1 ? 'b' : 'r');
   putchar('\n');
   return 0;
  }
 }
 puts("none");  // 起きない (塗り方はいつもある)
}
