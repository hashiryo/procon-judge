// 期待出力を作る参照実装。行と数を結ぶ二部多重グラフ (行 i と数 v の間に、行 i の中の v の個数だけ辺がある) から、増加路を
// 1 本ずつ探す素朴な方法 (Kuhn) で完全マッチングを取り、取った組を 1 列に並べて辺を減らすことを M 回繰り返す。ライブラリを
// include しない (期待出力のキャッシュの鍵はこのファイルの中身だけで決まる)。並べ方は checker.cpp が確かめる。
#include <cstdio>
#include <vector>
using namespace std;
int n, m;
vector<vector<int>> cnt;
vector<int> mr, vis;
int stamp = 0;
bool dfs(int i) {
  for (int v = 0; v < n; ++v)
    if (cnt[i][v] > 0 && vis[v] != stamp) {
      vis[v] = stamp;
      if (mr[v] < 0 || dfs(mr[v])) return mr[v] = i, true;
    }
  return false;
}
int main() {
  if (scanf("%d %d", &n, &m) != 2) return 1;
  cnt.assign(n, vector<int>(n, 0));
  for (int i = 0; i < n; ++i)
    for (int j = 0; j < m; ++j) {
      int a;
      if (scanf("%d", &a) != 1) return 1;
      ++cnt[i][a - 1];
    }
  vector<vector<int>> b(n, vector<int>(m));
  vis.assign(n, 0);
  for (int c = 0; c < m; ++c) {
    mr.assign(n, -1);
    for (int i = 0; i < n; ++i) {
      ++stamp;
      if (!dfs(i)) return puts("No"), 0;  // M 正則なので起きない
    }
    for (int v = 0; v < n; ++v) b[mr[v]][c] = v + 1, --cnt[mr[v]][v];
  }
  puts("Yes");
  for (int i = 0; i < n; ++i)
    for (int j = 0; j < m; ++j) printf("%d%c", b[i][j], j + 1 == m ? '\n' : ' ');
}
