// cf-1615-g の愚直解。0 の位置に 1 から n を全部試し、値 (隣り合う 2 つが等しい k の種類の数) が最大の配列を 1 つ出す。
// マッチングへの帰着を使わないので、参照実装とは別の考え方になる。n <= 8 で 0 が 6 個以下の入力でだけ使う (gen.py の seed 1000 以上)。
#include <cstdio>
#include <vector>
int main() {
 int n;
 if(scanf("%d", &n) != 1) return 1;
 std::vector<int> a(n), z;
 for(int i= 0; i < n; ++i) {
  if(scanf("%d", &a[i]) != 1) return 1;
  if(a[i] == 0) z.push_back(i);
 }
 std::vector<int> b= a, best;
 int bestv= -1;
 for(int i: z) b[i]= 1;
 for(;;) {
  std::vector<char> seen(n + 1, 0);
  int v= 0;
  for(int i= 0; i + 1 < n; ++i)
   if(b[i] == b[i + 1] && !seen[b[i]]) seen[b[i]]= 1, ++v;
  if(v > bestv) bestv= v, best= b;
  size_t k= 0;
  while(k < z.size() && b[z[k]] == n) b[z[k++]]= 1;
  if(k == z.size()) break;
  ++b[z[k]];
 }
 for(int i= 0; i < n; ++i) printf("%d%c", best[i], i + 1 == n ? '\n' : ' ');
}
