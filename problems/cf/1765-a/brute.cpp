// cf-1765-a の愚直解。文書の組の分け方 (集合の分割) を全部試し、どの組の中でも、見られる開発者の集合がどの 2 つも包含で
// 比べられる (片方がもう片方を含む) ものから、組の数が最小のものを選ぶ。組の中を集合の大きい順に並べて水準を付ける。
// 二部マッチングも Dilworth の定理も使わないので、提出とは別の考え方になる。文書が 8 個以下の入力でだけ使う
// (gen.py の seed 1000 以上)。
#include <algorithm>
#include <cstdio>
#include <string>
#include <vector>
int n, m, best_k;
std::vector<std::string> a;
std::vector<unsigned> col;
std::vector<int> cur, best;
bool cmp_ok(int j, int k) { return (col[j] & col[k]) == col[k] || (col[j] & col[k]) == col[j]; }
void rec(int j, int k) {
 if(k >= best_k) return;
 if(j == m) {
  best_k= k, best= cur;
  return;
 }
 for(int g= 0; g <= k; ++g) {
  bool ok= true;
  for(int x= 0; x < j && ok; ++x)
   if(cur[x] == g && !cmp_ok(x, j)) ok= false;
  if(!ok) continue;
  cur[j]= g;
  rec(j + 1, std::max(k, g + 1));
 }
}
int main() {
 if(scanf("%d %d", &n, &m) != 2) return 1;
 if(n > 32 || m > 10) return 1;
 a.resize(n);
 static char buf[64];
 for(auto& s: a) {
  if(scanf("%63s", buf) != 1) return 1;
  s= buf;
 }
 col.assign(m, 0);
 for(int i= 0; i < n; ++i)
  for(int j= 0; j < m; ++j)
   if(a[i][j] == '1') col[j]|= 1u << i;
 best_k= m + 1, cur.assign(m, -1);
 rec(0, 0);
 const int k= best_k;
 std::vector<int> lev(m);
 for(int g= 0; g < k; ++g) {
  std::vector<int> ds;
  for(int j= 0; j < m; ++j)
   if(best[j] == g) ds.push_back(j);
  std::sort(ds.begin(), ds.end(), [&](int x, int y) { return __builtin_popcount(col[x]) > __builtin_popcount(col[y]); });
  for(int p= 0; p < (int)ds.size(); ++p) lev[ds[p]]= p + 2;
 }
 printf("%d\n", k);
 for(int j= 0; j < m; ++j) printf("%d%c", best[j] + 1, j + 1 < m ? ' ' : '\n');
 for(int j= 0; j < m; ++j) printf("%d%c", lev[j], j + 1 < m ? ' ' : '\n');
 for(int i= 0; i < n; ++i) {
  std::vector<int> v(k, 1);
  for(int j= 0; j < m; ++j)
   if(a[i][j] == '1') v[best[j]]= std::max(v[best[j]], lev[j]);
  for(int g= 0; g < k; ++g) printf("%d%c", v[g], g + 1 < k ? ' ' : '\n');
 }
}
