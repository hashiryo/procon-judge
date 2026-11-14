// 愚直解。? のマスの塗り方を全部試し、辺で接する黒マスと白マスの組の数の最大を取る。? が 20 個ほどまで。
#include <cstdio>
#include <string>
#include <vector>
using namespace std;
int main() {
  int n;
  if (scanf("%d", &n) != 1) return 1;
  vector<string> c(n);
  for (auto& row : c) {
    char buf[128];
    if (scanf("%127s", buf) != 1) return 1;
    row = buf;
  }
  vector<int> qs;
  for (int i = 0; i < n * n; ++i)
    if (c[i / n][i % n] == '?') qs.push_back(i);
  int best = 0;
  for (int mask = 0; mask < (1 << qs.size()); ++mask) {
    vector<string> g = c;
    for (size_t k = 0; k < qs.size(); ++k) g[qs[k] / n][qs[k] % n] = mask >> k & 1 ? 'W' : 'B';
    int z = 0;
    for (int i = 0; i < n; ++i)
      for (int j = 0; j < n; ++j) {
        if (i + 1 < n && g[i][j] != g[i + 1][j]) ++z;
        if (j + 1 < n && g[i][j] != g[i][j + 1]) ++z;
      }
    if (z > best) best = z;
  }
  printf("%d\n", best);
}
