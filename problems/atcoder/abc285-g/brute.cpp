// 愚直解。上の行から順に、まだ覆っていないマスを 1 × 1 か、右か下へ伸ばした 1 × 2 で覆う敷き方を全部試す。1 のマスは
// 1 × 1 だけ、2 のマスは 1 × 2 だけ、相手のマスも 1 でないものだけにする。H, W が 5 ほどまで。
#include <cstdio>
#include <string>
#include <vector>
using namespace std;
int h, w;
vector<string> c;
vector<vector<char>> used;
bool rec(int p) {
  if (p == h * w) return true;
  const int i = p / w, j = p % w;
  if (used[i][j]) return rec(p + 1);
  used[i][j] = 1;
  if (c[i][j] != '2' && rec(p + 1)) return true;
  if (c[i][j] != '1') {
    if (j + 1 < w && !used[i][j + 1] && c[i][j + 1] != '1') {
      used[i][j + 1] = 1;
      if (rec(p + 1)) return true;
      used[i][j + 1] = 0;
    }
    if (i + 1 < h && !used[i + 1][j] && c[i + 1][j] != '1') {
      used[i + 1][j] = 1;
      if (rec(p + 1)) return true;
      used[i + 1][j] = 0;
    }
  }
  used[i][j] = 0;
  return false;
}
int main() {
  if (scanf("%d %d", &h, &w) != 2) return 1;
  c.resize(h);
  for (auto& row : c) {
    char buf[64];
    if (scanf("%63s", buf) != 1) return 1;
    row = buf;
  }
  used.assign(h, vector<char>(w, 0));
  puts(rec(0) ? "Yes" : "No");
}
