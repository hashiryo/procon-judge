// loj-6002 のチェッカ。引数は入力、提出の出力、期待出力の順 (pj の約束)。
//
// 提出の出力は、1 行に 1 本ずつ道を書き、最後の行に道の本数を書いたもの。最後の行の本数が期待出力の最後の数と同じで、
// ほかの行がちょうどその本数あり、どの行も隣り合う頂点が入力の辺で結ばれ、どの頂点もちょうど 1 回出てくることを確かめる。
#include <cstdio>
#include <fstream>
#include <set>
#include <sstream>
#include <string>
#include <utility>
#include <vector>
using namespace std;
int main(int argc, char** argv) {
  if (argc < 4) return 2;
  FILE* in = fopen(argv[1], "r");
  if (!in) return 2;
  int n, m;
  if (fscanf(in, "%d %d", &n, &m) != 2) return 2;
  set<pair<int, int>> es;
  for (int i = 0; i < m; ++i) {
    int u, v;
    if (fscanf(in, "%d %d", &u, &v) != 2) return 2;
    es.insert({u, v});
  }
  long long want = -1;
  {
    ifstream ans(argv[3]);
    string tok;
    while (ans >> tok) want = stoll(tok);
  }
  if (want < 0) return 2;
  ifstream out(argv[2]);
  vector<vector<long long>> lines;
  for (string s; getline(out, s);) {
    istringstream is(s);
    vector<long long> row;
    for (long long x; is >> x;) row.push_back(x);
    string rest;
    if (!is.eof()) return fprintf(stderr, "数でない文字があります\n"), 1;
    if (!row.empty()) lines.push_back(row);
  }
  if (lines.empty()) return fprintf(stderr, "出力が空です\n"), 1;
  if (lines.back().size() != 1) return fprintf(stderr, "最後の行が本数 1 つではありません\n"), 1;
  const long long got = lines.back()[0];
  if (got != want) return fprintf(stderr, "本数が %lld ですが、期待出力は %lld です\n", got, want), 1;
  lines.pop_back();
  if ((long long)lines.size() != got)
    return fprintf(stderr, "道の行が %zu 本ありますが、本数は %lld です\n", lines.size(), got), 1;
  vector<int> seen(n + 1, 0);
  for (size_t k = 0; k < lines.size(); ++k) {
    const auto& p = lines[k];
    for (size_t i = 0; i < p.size(); ++i) {
      if (p[i] < 1 || p[i] > n) return fprintf(stderr, "%zu 本目の道に範囲の外の頂点 %lld があります\n", k + 1, p[i]), 1;
      if (seen[p[i]]++) return fprintf(stderr, "頂点 %lld が 2 回出てきます\n", p[i]), 1;
      if (i && !es.count({(int)p[i - 1], (int)p[i]}))
        return fprintf(stderr, "%zu 本目の道の %lld から %lld への辺がありません\n", k + 1, p[i - 1], p[i]), 1;
    }
  }
  for (int v = 1; v <= n; ++v)
    if (!seen[v]) return fprintf(stderr, "頂点 %d がどの道にもありません\n", v), 1;
  return 0;
}
