// luogu-P4298 の愚直解。到達可能性を Floyd-Warshall で求め、頂点の部分集合を全部試して反鎖を数える。K は反鎖の大きさの
// 最大、2 行目は大きさ K の反鎖のうち最初に見つかったもの、3 行目は頂点ごとに、その頂点を含む大きさ K の反鎖があるか。
// 二部マッチングも Dilworth の定理も使わないので、提出とは別の考え方になる。頂点が 14 個以下の入力でだけ使う
// (gen.py の seed 1000 以上)。
#include <cstdio>
#include <string>
#include <vector>
int main() {
 int n, m;
 if(scanf("%d %d", &n, &m) != 2) return 1;
 if(n > 16) return 1;
 std::vector<std::vector<char>> r(n, std::vector<char>(n, 0));
 for(int i= 0, u, v; i < m; ++i) {
  if(scanf("%d %d", &u, &v) != 2) return 1;
  r[u - 1][v - 1]= 1;
 }
 for(int k= 0; k < n; ++k)
  for(int i= 0; i < n; ++i)
   for(int j= 0; j < n; ++j)
    if(r[i][k] && r[k][j]) r[i][j]= 1;
 std::vector<int> cmp(n, 0);  // 比べられる頂点のビット
 for(int i= 0; i < n; ++i)
  for(int j= 0; j < n; ++j)
   if(r[i][j] || r[j][i]) cmp[i]|= 1 << j;
 int best= 0, first= 0, can= 0;
 for(int s= 1; s < (1 << n); ++s) {
  bool ok= true;
  for(int i= 0; i < n && ok; ++i)
   if((s >> i & 1) && (cmp[i] & s)) ok= false;
  if(!ok) continue;
  const int c= __builtin_popcount(s);
  if(c > best) best= c, first= s, can= s;
  else if(c == best) can|= s;
 }
 std::string a(n, '0'), b(n, '0');
 for(int i= 0; i < n; ++i) a[i]= first >> i & 1 ? '1' : '0', b[i]= can >> i & 1 ? '1' : '0';
 printf("%d\n%s\n%s\n", best, a.c_str(), b.c_str());
}
