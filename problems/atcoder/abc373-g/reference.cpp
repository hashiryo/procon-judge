// 期待出力を作る参照実装。浮動小数点数を使わない分割統治で、交差しない組を作る。点の集まり (P と Q が同じ数) のうち最も下
// (同じなら最も左) の点 o を取り、残りを o のまわりの偏角の順に並べる。o と同じ色を +1、違う色を -1 として先頭から足して
// いくと、和は 0 から始まって -1 で終わるので、和が初めて 0 から -1 に下がる点 q は o と違う色で、q より前は数が釣り合う。
// o と q を組ませ、q より前と後ろをそれぞれ同じように解く。前と後ろは直線 oq の両側に分かれるので、線分は交わらない。
// ライブラリを include しない (期待出力のキャッシュの鍵はこのファイルの中身だけで決まる)。
#include <algorithm>
#include <cstdio>
#include <vector>
using namespace std;
using i64 = long long;
int n;
vector<i64> X, Y;
vector<int> mate;
i64 cross(int o, int a, int b) { return (X[a] - X[o]) * (Y[b] - Y[o]) - (Y[a] - Y[o]) * (X[b] - X[o]); }
void solve(vector<int> ids) {
  if (ids.empty()) return;
  int o = ids[0];
  for (int v : ids)
    if (Y[v] < Y[o] || (Y[v] == Y[o] && X[v] < X[o])) o = v;
  vector<int> rest;
  for (int v : ids)
    if (v != o) rest.push_back(v);
  sort(rest.begin(), rest.end(), [&](int a, int b) { return cross(o, a, b) > 0; });
  const bool co = o < n;
  int bal = 0;
  for (size_t k = 0; k < rest.size(); ++k) {
    const bool cq = rest[k] < n;
    if (cq != co && bal == 0) {
      const int p = co ? o : rest[k], q = co ? rest[k] : o;
      mate[p] = q - n;
      solve(vector<int>(rest.begin(), rest.begin() + k));
      solve(vector<int>(rest.begin() + k + 1, rest.end()));
      return;
    }
    bal += cq == co ? 1 : -1;
  }
}
int main() {
  if (scanf("%d", &n) != 1) return 1;
  X.resize(2 * n), Y.resize(2 * n);
  for (int i = 0; i < 2 * n; ++i) if (scanf("%lld %lld", &X[i], &Y[i]) != 2) return 1;
  mate.assign(n, -1);
  vector<int> all(2 * n);
  for (int i = 0; i < 2 * n; ++i) all[i] = i;
  solve(all);
  for (int i = 0; i < n; ++i) printf("%d%c", mate[i] + 1, i + 1 == n ? '\n' : ' ');
}
