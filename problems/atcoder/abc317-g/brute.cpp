// 愚直解。N, M ≤ 3 の入力で、行ごとの並べ替えを辞書順に全部試し、各列が 1 から N を 1 つずつ含む最初のものを出す。
#include <algorithm>
#include <cstdio>
#include <vector>
using namespace std;
int n, m;
vector<vector<int>> a;
bool ok() {
  for (int j = 0; j < m; ++j) {
    vector<int> seen(n + 1, 0);
    for (int i = 0; i < n; ++i)
      if (seen[a[i][j]]++) return false;
  }
  return true;
}
bool rec(int i) {
  if (i == n) return ok();
  sort(a[i].begin(), a[i].end());
  do
    if (rec(i + 1)) return true;
  while (next_permutation(a[i].begin(), a[i].end()));
  return false;
}
int main() {
  if (scanf("%d %d", &n, &m) != 2) return 1;
  a.assign(n, vector<int>(m));
  for (auto& row : a)
    for (int& x : row) if (scanf("%d", &x) != 1) return 1;
  if (!rec(0)) return puts("No"), 0;
  puts("Yes");
  for (auto& row : a)
    for (int j = 0; j < m; ++j) printf("%d%c", row[j], j + 1 == m ? '\n' : ' ');
}
