// abc198-f の愚直解。6 面の数を小さい順に並べた組 a1 <= ... <= a6 (和が S) を全部たどり、組ごとに、
// その 6 個の数を面に置く置き方を、回転で重なるものを 1 つと数えて足す。置き方の数は、組の中の
// どの数どうしが等しいかだけで決まるので、その形ごとに 1 度だけ、並べ方を全部作って 24 個の回転で
// 最小のものに揃え、異なるものを数えて覚えておく。バーンサイドの補題も母関数も漸化式も使わないので、
// 提出とは別の考え方になる。組の数は S^5 に比例するので、小さい S でだけ使う。
#include <algorithm>
#include <array>
#include <cstdio>
#include <set>
#include <vector>

using Faces = std::array<int, 6>;  // 上、下、前、後、左、右の順

// 回転後の面 f の数は、回転前の面 p[f] の数。2 つの 90 度回転から全部の回転を作る。
std::vector<Faces> rotations() {
  const Faces yaw = {0, 1, 4, 5, 3, 2};    // 上下の軸まわり (前 -> 右 -> 後 -> 左 -> 前)
  const Faces pitch = {3, 2, 0, 1, 4, 5};  // 左右の軸まわり (上 -> 前 -> 下 -> 後 -> 上)
  std::vector<Faces> group = {{0, 1, 2, 3, 4, 5}};
  for (size_t i = 0; i < group.size(); ++i)
    for (const Faces &g : {yaw, pitch}) {
      Faces next;
      for (int f = 0; f < 6; ++f) next[f] = group[i][g[f]];
      if (std::find(group.begin(), group.end(), next) == group.end()) group.push_back(next);
    }
  return group;
}

int main() {
  long long s;
  if (scanf("%lld", &s) != 1) return 1;
  const std::vector<Faces> group = rotations();
  if (group.size() != 24) return 2;

  // ways[mask]: 隣どうしが等しいかを 5 ビットで表した形の、回転を除いた置き方の数。
  long long ways[32];
  for (int mask = 0; mask < 32; ++mask) {
    Faces values;
    values[0] = 0;
    for (int i = 1; i < 6; ++i) values[i] = values[i - 1] + (mask >> (i - 1) & 1 ? 0 : 1);
    std::set<Faces> seen;
    Faces a = values;
    do {
      Faces best = a;
      for (const Faces &g : group) {
        Faces b;
        for (int f = 0; f < 6; ++f) b[f] = a[g[f]];
        best = std::min(best, b);
      }
      seen.insert(best);
    } while (std::next_permutation(a.begin(), a.end()));
    ways[mask] = seen.size();
  }

  long long count = 0;
  for (long long a1 = 1; 6 * a1 <= s; ++a1)
    for (long long a2 = a1; a1 + 5 * a2 <= s; ++a2)
      for (long long a3 = a2; a1 + a2 + 4 * a3 <= s; ++a3)
        for (long long a4 = a3; a1 + a2 + a3 + 3 * a4 <= s; ++a4)
          for (long long a5 = a4; a1 + a2 + a3 + a4 + 2 * a5 <= s; ++a5) {
            long long a6 = s - a1 - a2 - a3 - a4 - a5;
            int mask = (a1 == a2) | (a2 == a3) << 1 | (a3 == a4) << 2 | (a4 == a5) << 3 | (a5 == a6) << 4;
            count += ways[mask];
          }
  printf("%lld\n", count % 998244353);
}
