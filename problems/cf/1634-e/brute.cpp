// cf-1634-e の愚直解。配列ごとに、要素の半分を L に選ぶ選び方を全部試し、L と R が多重集合として等しくなるものを 1 つ出す。無ければ
// NO。Euler 分割も値の偶奇の条件も使わないので、提出とは別の考え方になる。要素の数の和が 12 以下の入力でだけ使う
// (gen.py の seed 1000 以上)。
#include <cstdio>
#include <map>
#include <vector>
int main() {
 int m;
 if(scanf("%d", &m) != 1) return 1;
 std::vector<std::vector<int>> a(m);
 for(auto& v: a) {
  int n;
  if(scanf("%d", &n) != 1) return 1;
  v.resize(n);
  for(int& x: v)
   if(scanf("%d", &x) != 1) return 1;
 }
 // 配列ごとの、L に入れる要素の集合 (ビット) で、ちょうど半分のもの。
 std::vector<std::vector<int>> opts(m);
 for(int i= 0; i < m; ++i)
  for(int s= 0; s < (1 << a[i].size()); ++s)
   if(__builtin_popcount(s) * 2 == (int)a[i].size()) opts[i].push_back(s);
 std::vector<int> pick(m, 0);
 for(;;) {
  std::map<int, int> d;
  for(int i= 0; i < m; ++i)
   for(size_t j= 0; j < a[i].size(); ++j) d[a[i][j]]+= (opts[i][pick[i]] >> j & 1) ? 1 : -1;
  bool ok= true;
  for(auto& [v, c]: d) ok&= c == 0;
  if(ok) {
   puts("YES");
   for(int i= 0; i < m; ++i) {
    for(size_t j= 0; j < a[i].size(); ++j) putchar((opts[i][pick[i]] >> j & 1) ? 'L' : 'R');
    putchar('\n');
   }
   return 0;
  }
  int i= 0;
  while(i < m && pick[i] + 1 == (int)opts[i].size()) pick[i++]= 0;
  if(i == m) break;
  ++pick[i];
 }
 puts("NO");
}
