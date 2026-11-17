// luogu-P4298 のチェッカ。引数は入力、提出の出力、期待出力の順 (pj の約束)。
//
// 1 行目の K と 3 行目の 01 の列は答えが 1 つに決まるので、期待出力と同じことを確かめる。2 行目は最大反鎖の 1 つで、どれでも
// よいので、長さ N の 01 の列で 1 がちょうど K 個あり、選んだどの 2 つの間にも水の流れる道が無いことを確かめる。
#include <bitset>
#include <cstdio>
#include <string>
#include <vector>
using namespace std;
static bool read_token(FILE* f, string& s) {
  char buf[1024];
  if (fscanf(f, "%1023s", buf) != 1) return false;
  s = buf;
  return true;
}
int main(int argc, char** argv) {
  if (argc < 4) return 2;
  FILE* in = fopen(argv[1], "r");
  FILE* out = fopen(argv[2], "r");
  FILE* ans = fopen(argv[3], "r");
  if (!in || !out || !ans) return 2;
  int n, m;
  if (fscanf(in, "%d %d", &n, &m) != 2 || n > 200) return 2;
  vector<vector<int>> g(n);
  for (int i = 0; i < m; ++i) {
    int u, v;
    if (fscanf(in, "%d %d", &u, &v) != 2) return 2;
    g[u - 1].push_back(v - 1);
  }
  // 頂点ごとに、そこから届く頂点を BFS で数える。
  vector<bitset<200>> reach(n);
  for (int s = 0; s < n; ++s) {
    vector<int> q{s};
    vector<char> vis(n, 0);
    vis[s] = 1;
    for (size_t i = 0; i < q.size(); ++i)
      for (int w : g[q[i]])
        if (!vis[w]) vis[w] = 1, reach[s][w] = 1, q.push_back(w);
  }
  string wk, w2, w3, gk, g2, g3, extra;
  if (!read_token(ans, wk) || !read_token(ans, w2) || !read_token(ans, w3)) return 2;
  if (!read_token(out, gk)) return fprintf(stderr, "K を読めません\n"), 1;
  if (gk != wk) return fprintf(stderr, "K が %s ですが、期待出力は %s です\n", gk.c_str(), wk.c_str()), 1;
  if (!read_token(out, g2) || !read_token(out, g3)) return fprintf(stderr, "2 行目か 3 行目を読めません\n"), 1;
  if (read_token(out, extra)) return fprintf(stderr, "余計な出力があります\n"), 1;
  if ((int)g2.size() != n) return fprintf(stderr, "2 行目の長さが %zu で、N は %d です\n", g2.size(), n), 1;
  int ones = 0;
  vector<int> chosen;
  for (int i = 0; i < n; ++i) {
    if (g2[i] != '0' && g2[i] != '1') return fprintf(stderr, "2 行目に 0 と 1 以外の文字があります\n"), 1;
    if (g2[i] == '1') ++ones, chosen.push_back(i);
  }
  if (to_string(ones) != wk) return fprintf(stderr, "2 行目の 1 が %d 個で、K は %s です\n", ones, wk.c_str()), 1;
  for (int a : chosen)
    for (int b : chosen)
      if (reach[a][b]) return fprintf(stderr, "岔口 %d から %d へ水が流れます\n", a + 1, b + 1), 1;
  if (g3 != w3) return fprintf(stderr, "3 行目が %s ですが、期待出力は %s です\n", g3.c_str(), w3.c_str()), 1;
  return 0;
}
