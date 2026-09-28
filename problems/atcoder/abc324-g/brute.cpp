// abc324-g の愚直解。数列を配列のまま持ち、操作どおりに前から x 個を残す (t = 1)、x 以下の値を残す (t = 2)
// ように分けて、取り除いた要素を新しい配列にする。数列を位置と値の長方形で表さず、WaveletMatrix で
// 数えもしないので、提出とは別の考え方になる。1 回の操作が数列の長さぶんかかるので、小さい入力でだけ使う。
#include <cstdio>
#include <vector>

int main() {
  int n, q;
  if (scanf("%d", &n) != 1) return 1;
  std::vector<std::vector<int>> seqs(1, std::vector<int>(n));
  for (auto &v : seqs[0])
    if (scanf("%d", &v) != 1) return 1;
  if (scanf("%d", &q) != 1) return 1;
  for (int i = 1; i <= q; ++i) {
    int t, s, x;
    if (scanf("%d %d %d", &t, &s, &x) != 3) return 1;
    std::vector<int> keep, removed;
    for (size_t j = 0; j < seqs[s].size(); ++j) {
      int v = seqs[s][j];
      bool stays = t == 1 ? (int)j < x : v <= x;
      (stays ? keep : removed).push_back(v);
    }
    seqs[s] = keep;
    seqs.push_back(removed);
    printf("%zu\n", removed.size());
  }
}
