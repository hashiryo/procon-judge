// abc321-g の愚直解。赤い端子 i と青い端子 p(i) をつなぐ置換 p を M! 通り全部試し、UnionFind で
// 連結成分を数えて足し、M! で割る。部分集合の数え上げ (set power series の log) を使わないので、
// 提出とは別の考え方になる。O(M! M) なので、M ≤ 9 でだけ使う。
#include <algorithm>
#include <cstdio>
#include <numeric>
#include <vector>

const long long P = 998244353;

long long pow_mod(long long x, long long e) {
  long long r = 1;
  for (x %= P; e; e >>= 1, x = x * x % P)
    if (e & 1) r = r * x % P;
  return r;
}

int find(std::vector<int>& par, int v) { return par[v] == v ? v : par[v] = find(par, par[v]); }

int main() {
  int n, m;
  if (scanf("%d %d", &n, &m) != 2) return 1;
  std::vector<int> r(m), b(m);
  for (int& x : r)
    if (scanf("%d", &x) != 1) return 1;
  for (int& x : b)
    if (scanf("%d", &x) != 1) return 1;
  std::vector<int> p(m);
  std::iota(p.begin(), p.end(), 0);
  long long total = 0, ways = 0;
  do {
    std::vector<int> par(n + 1);
    std::iota(par.begin(), par.end(), 0);
    int comps = n;
    for (int i = 0; i < m; ++i) {
      int x = find(par, r[i]), y = find(par, b[p[i]]);
      if (x != y) par[x] = y, --comps;
    }
    total += comps, ++ways;
  } while (std::next_permutation(p.begin(), p.end()));
  printf("%lld\n", total % P * pow_mod(ways % P, P - 2) % P);
}
