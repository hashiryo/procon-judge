// cf-1630-f の愚直解。テストケースごとに、残す頂点の部分集合を全部試し、残したグラフ (割り切れる組を辺にしたもの) が 2 色で
// 塗れるかを BFS で確かめて、残せる頂点の最大を n から引く。鎖や反鎖の言い換えを使わないので、提出とは別の考え方になる。
// 頂点が 14 個以下のテストケースでだけ使う (gen.py の seed 1000 以上)。
#include <cstdio>
#include <string>
#include <vector>
int main() {
 int t;
 if(scanf("%d", &t) != 1) return 1;
 std::string out;
 while(t--) {
  int n;
  if(scanf("%d", &n) != 1) return 1;
  std::vector<int> a(n);
  for(int& x: a)
   if(scanf("%d", &x) != 1) return 1;
  if(n > 16) return 1;
  std::vector<int> adj(n, 0);
  for(int u= 0; u < n; ++u)
   for(int v= 0; v < n; ++v)
    if(u != v && (a[v] % a[u] == 0 || a[u] % a[v] == 0)) adj[u]|= 1 << v;
  int best= 0;
  for(int s= 0; s < (1 << n); ++s) {
   if(__builtin_popcount(s) <= best) continue;
   std::vector<int> col(n, -1);
   bool ok= true;
   for(int r= 0; r < n && ok; ++r) {
    if(!(s >> r & 1) || col[r] >= 0) continue;
    std::vector<int> q{r};
    col[r]= 0;
    for(size_t i= 0; i < q.size() && ok; ++i)
     for(int w= 0; w < n; ++w)
      if((adj[q[i]] & s) >> w & 1) {
       if(col[w] < 0) col[w]= col[q[i]] ^ 1, q.push_back(w);
       else if(col[w] == col[q[i]]) ok= false;
      }
   }
   if(ok) best= __builtin_popcount(s);
  }
  out+= std::to_string(n - best), out+= '\n';
 }
 fputs(out.c_str(), stdout);
}
