// arc153-b の愚直解。操作のたびに 4 つの長方形の中身を実際に 180 度回した盤面を作り直す。
// 行の並びと列の並びに分けず、平衡二分木の区間反転も使わないので、提出とは別の考え方になる。
// 1 回の操作が O(HW) なので、盤面と Q が小さいときだけ使う。
#include <cstdio>
#include <string>
#include <vector>

int main() {
  int h, w, q;
  if (scanf("%d %d", &h, &w) != 2) return 1;
  std::vector<std::string> grid(h);
  for (auto& row : grid) {
    static char buf[1 << 20];
    if (scanf("%s", buf) != 1) return 1;
    row = buf;
  }
  if (scanf("%d", &q) != 1) return 1;
  while (q--) {
    int a, b;
    if (scanf("%d %d", &a, &b) != 2) return 1;
    std::vector<std::string> next = grid;
    int rows[3] = {0, a, h}, cols[3] = {0, b, w};
    for (int i = 0; i < 2; ++i)
      for (int j = 0; j < 2; ++j) {
        int r0 = rows[i], r1 = rows[i + 1], c0 = cols[j], c1 = cols[j + 1];
        // 長方形 [r0, r1) x [c0, c1) の中で、上から i 番目・左から j 番目を下から i 番目・右から j 番目へ
        for (int r = r0; r < r1; ++r)
          for (int c = c0; c < c1; ++c) next[r0 + r1 - 1 - r][c0 + c1 - 1 - c] = grid[r][c];
      }
    grid = next;
  }
  for (auto& row : grid) puts(row.c_str());
}
