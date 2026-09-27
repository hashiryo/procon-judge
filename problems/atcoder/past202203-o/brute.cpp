// past202203-o の愚直解。P_A + P_B が 3 の倍数かは P を 3 で割った余りだけで決まり、1 から N の余りは
// 0 が N / 3 個、1 が (N + 2) / 3 個、2 が (N + 1) / 3 個ある。頂点に 1 つずつ余りを決めていき、個数を
// 使い切らないか、決めた頂点どうしの組がどれも 0 か 3 の倍数になるかを見ながら全部試す。
// 連結成分も 2 部グラフの判定も DP も使わないので、提出とは別の考え方になる。小さい N でだけ使う。
#include <cstdio>
#include <vector>

int n;
std::vector<std::vector<int>> earlier;  // earlier[v]: v より番号の小さい隣の頂点
std::vector<int> residue;
int left[3];

bool search(int v) {
  if (v == n) return true;
  for (int r = 0; r < 3; ++r) {
    if (left[r] == 0) continue;
    bool ok = true;
    for (int u : earlier[v])
      if ((residue[u] + r) % 3 != 0) ok = false;
    if (!ok) continue;
    residue[v] = r, --left[r];
    if (search(v + 1)) return true;
    ++left[r];
  }
  return false;
}

int main() {
  int m;
  if (scanf("%d %d", &n, &m) != 2) return 1;
  earlier.assign(n, {});
  residue.assign(n, -1);
  for (int i = 0; i < m; ++i) {
    int a, b;
    if (scanf("%d %d", &a, &b) != 2) return 1;
    earlier[b - 1].push_back(a - 1);  // a < b
  }
  left[0] = n / 3, left[1] = (n + 2) / 3, left[2] = (n + 1) / 3;
  puts(search(0) ? "Yes" : "No");
}
