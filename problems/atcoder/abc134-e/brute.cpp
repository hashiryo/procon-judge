// abc134-e の愚直解。i < j かつ A_i < A_j のとき i から j へ辺を張った DAG を作ると、同じ色の部分列は
// この DAG の道になるので、色の数の最小は最小パス被覆で、N から二部マッチングの最大を引いたものになる。
// マッチングは増加路を 1 本ずつ探して求める。最長の広義単調減少部分列 (Dilworth の定理) も二分探索も
// 使わないので、提出とは別の考え方になる。O(N^3) なので、小さい入力でだけ使う。
#include <cstdio>
#include <vector>

int n;
std::vector<long long> a;
std::vector<int> match_to;  // 右側の頂点 j に繋いだ左側の頂点
std::vector<char> seen;

bool augment(int i) {
  for (int j = i + 1; j < n; ++j) {
    if (a[i] >= a[j] || seen[j]) continue;
    seen[j] = 1;
    if (match_to[j] < 0 || augment(match_to[j])) {
      match_to[j] = i;
      return true;
    }
  }
  return false;
}

int main() {
  if (scanf("%d", &n) != 1) return 1;
  a.resize(n);
  for (auto &v : a)
    if (scanf("%lld", &v) != 1) return 1;
  match_to.assign(n, -1);
  int matching = 0;
  for (int i = 0; i < n; ++i) {
    seen.assign(n, 0);
    if (augment(i)) ++matching;
  }
  printf("%d\n", n - matching);
}
