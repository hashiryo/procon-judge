// atcoder-abc239-g のチェッカ。引数は入力、提出の出力、期待出力の順 (pj の約束)。
//
// 提出の C が期待出力の最適値と一致し、k 個の頂点が 2 以上 N - 1 以下で重複が無く、その費用の和が C で、それらを除くと
// 1 から N へ行けないことを確かめる。壁を置く頂点の集合はどれでもよいので、期待出力からは最適値だけを読む。
#include <cstdio>
#include <vector>
using namespace std;
using i64 = long long;
int main(int argc, char** argv) {
  if (argc < 4) return 2;
  FILE* in = fopen(argv[1], "r");
  FILE* out = fopen(argv[2], "r");
  FILE* ans = fopen(argv[3], "r");
  if (!in || !out || !ans) return 2;
  int n, m;
  if (fscanf(in, "%d %d", &n, &m) != 2) return 2;
  vector<vector<int>> adj(n + 1);
  for (int i = 0; i < m; ++i) {
    int a, b;
    if (fscanf(in, "%d %d", &a, &b) != 2) return 2;
    adj[a].push_back(b), adj[b].push_back(a);
  }
  vector<i64> c(n + 1);
  for (int i = 1; i <= n; ++i) if (fscanf(in, "%lld", &c[i]) != 1) return 2;
  i64 opt;
  if (fscanf(ans, "%lld", &opt) != 1) return 2;
  i64 got;
  int k;
  if (fscanf(out, "%lld %d", &got, &k) != 2) return fprintf(stderr, "C と k を読めません\n"), 1;
  if (got != opt) return fprintf(stderr, "C = %lld ですが、最適値は %lld です\n", got, opt), 1;
  if (k < 0 || k > n - 2) return fprintf(stderr, "k = %d が範囲の外です\n", k), 1;
  vector<char> wall(n + 1, 0);
  i64 sum = 0;
  for (int i = 0; i < k; ++i) {
    int p;
    if (fscanf(out, "%d", &p) != 1) return fprintf(stderr, "%d 個目の頂点を読めません\n", i + 1), 1;
    if (p < 2 || p > n - 1) return fprintf(stderr, "頂点 %d には壁を置けません\n", p), 1;
    if (wall[p]) return fprintf(stderr, "頂点 %d が 2 回出ています\n", p), 1;
    wall[p] = 1, sum += c[p];
  }
  int extra;
  if (fscanf(out, "%d", &extra) == 1) return fprintf(stderr, "k 個の頂点のあとに余計な出力があります\n"), 1;
  if (sum != got) return fprintf(stderr, "壁の費用の和は %lld で、C = %lld と違います\n", sum, got), 1;
  vector<char> vis(n + 1, 0);
  vector<int> st{1};
  vis[1] = 1;
  while (!st.empty()) {
    int u = st.back();
    st.pop_back();
    for (int v : adj[u]) if (!vis[v] && !wall[v]) vis[v] = 1, st.push_back(v);
  }
  if (vis[n]) return fprintf(stderr, "壁を置いても 1 から N へ行けます\n"), 1;
  return 0;
}
