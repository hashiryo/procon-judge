// 愚直解。提出ではない。seed 1000 以上の小さい入力 (n, q <= 40) だけを想定してよい。
// 更新は配列にそのまま反映し、クエリごとに区間を走査して異なる値を数える。
// 出力の形式は参照実装と同じ (クエリの順に 1 行ずつ)。
#include <cstdio>
#include <set>
#include <vector>
using namespace std;

int main() {
 int n, q;
 if(scanf("%d %d", &n, &q) != 2) return 1;
 vector<int> a(n);
 for(int i= 0; i < n; ++i) {
  if(scanf("%d", &a[i]) != 1) return 1;
 }
 for(int i= 0; i < q; ++i) {
  int type, x, y;
  if(scanf("%d %d %d", &type, &x, &y) != 3) return 1;
  if(type == 0) {
   a[x - 1]= y;
  } else {
   set<int> seen;
   for(int j= x - 1; j < y; ++j) seen.insert(a[j]);
   printf("%d\n", (int)seen.size());
  }
 }
 return 0;
}
