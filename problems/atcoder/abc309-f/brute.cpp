// abc309-f の愚直解。全部の組 (i, j) について箱 j の向きを 6 通り試し、3 辺とも箱 i より長くなる向きがあるかを調べる。
// 3 辺を並べ替えてから KDTree や SegmentTree_2D で探すことはしないので、提出とは別の考え方になる。
// O(N^2) なので小さい入力でだけ使う。
#include <algorithm>
#include <array>
#include <cstdio>
#include <vector>

int main() {
  int n;
  if (scanf("%d", &n) != 1) return 1;
  std::vector<std::array<long long, 3>> box(n);
  for (auto& b : box)
    if (scanf("%lld %lld %lld", &b[0], &b[1], &b[2]) != 3) return 1;
  bool found = false;
  for (int i = 0; i < n && !found; ++i)
    for (int j = 0; j < n && !found; ++j) {
      if (i == j) continue;
      std::array<int, 3> p = {0, 1, 2};
      do {
        if (box[i][0] < box[j][p[0]] && box[i][1] < box[j][p[1]] && box[i][2] < box[j][p[2]]) found = true;
      } while (!found && std::next_permutation(p.begin(), p.end()));
    }
  puts(found ? "Yes" : "No");
}
