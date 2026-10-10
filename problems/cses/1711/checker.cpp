// cses-1711 のチェッカ。引数は入力、提出の出力、期待出力の順 (pj の約束)。
//
// 道の数 k が期待出力の最適値と一致し、k 本の道がそれぞれ 1 で始まって n で終わり、隣り合う部屋の間に転送装置があり、
// どの転送装置も全体で 1 回までしか使わないことを確かめる。部屋の再訪は許す。道の組はどれでもよいので、期待出力からは
// 最適値だけを読む。
#include <cstdio>
#include <map>
#include <vector>
using namespace std;
int main(int argc, char** argv) {
  if (argc < 4) return 2;
  FILE* in = fopen(argv[1], "r");
  FILE* out = fopen(argv[2], "r");
  FILE* ans = fopen(argv[3], "r");
  if (!in || !out || !ans) return 2;
  int n, m;
  if (fscanf(in, "%d %d", &n, &m) != 2) return 2;
  map<pair<int, int>, int> id;
  for (int i = 0; i < m; ++i) {
    int a, b;
    if (fscanf(in, "%d %d", &a, &b) != 2) return 2;
    id[{a, b}] = i;
  }
  int opt;
  if (fscanf(ans, "%d", &opt) != 1) return 2;
  int k;
  if (fscanf(out, "%d", &k) != 1) return fprintf(stderr, "道の数を読めません\n"), 1;
  if (k != opt) return fprintf(stderr, "道の数 %d が最適値 %d と違います\n", k, opt), 1;
  vector<char> used(m, 0);
  for (int r = 1; r <= k; ++r) {
    int c;
    if (fscanf(out, "%d", &c) != 1 || c < 2 || c > m + 1) return fprintf(stderr, "%d 本目の長さが正しくありません\n", r), 1;
    vector<int> room(c);
    for (auto& x : room) if (fscanf(out, "%d", &x) != 1) return fprintf(stderr, "%d 本目の部屋を読めません\n", r), 1;
    if (room[0] != 1 || room[c - 1] != n) return fprintf(stderr, "%d 本目が 1 で始まり n で終わっていません\n", r), 1;
    for (int i = 0; i + 1 < c; ++i) {
      auto itr = id.find({room[i], room[i + 1]});
      if (itr == id.end()) return fprintf(stderr, "%d 本目の %d から %d への転送装置がありません\n", r, room[i], room[i + 1]), 1;
      if (used[itr->second]) return fprintf(stderr, "%d から %d への転送装置を 2 回使っています\n", room[i], room[i + 1]), 1;
      used[itr->second] = 1;
    }
  }
  int extra;
  if (fscanf(out, "%d", &extra) == 1) return fprintf(stderr, "k 本の道のあとに余計な出力があります\n"), 1;
  return 0;
}
