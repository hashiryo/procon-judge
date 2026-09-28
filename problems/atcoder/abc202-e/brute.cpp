// abc202-e の愚直解。質問ごとに U から子をたどって部分木の頂点を全部見て、深さが D のものを数える。
// 重軽分解の番号付けも WaveletMatrix もしないので、提出とは別の考え方になる。O(NQ) なので、
// 小さい入力でだけ使う。
#include <cstdio>
#include <vector>

int main() {
  int n;
  if (scanf("%d", &n) != 1) return 1;
  std::vector<std::vector<int>> children(n + 1);
  std::vector<int> depth(n + 1, 0);
  for (int i = 2; i <= n; ++i) {
    int p;
    if (scanf("%d", &p) != 1) return 1;
    children[p].push_back(i);
    depth[i] = depth[p] + 1;  // P_i < i なので親の深さは決まっている
  }
  int q;
  if (scanf("%d", &q) != 1) return 1;
  while (q--) {
    int u, d;
    if (scanf("%d %d", &u, &d) != 2) return 1;
    int count = 0;
    std::vector<int> stack = {u};
    while (!stack.empty()) {
      int v = stack.back();
      stack.pop_back();
      if (depth[v] == d) ++count;
      for (int c : children[v]) stack.push_back(c);
    }
    printf("%d\n", count);
  }
}
