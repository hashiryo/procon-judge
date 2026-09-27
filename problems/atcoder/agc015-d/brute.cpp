// agc015-d の愚直解。A から B までの数 x を順に見て、それまでに OR で作れる数の集合 S を S ∪ {x} ∪ {s | x : s ∈ S}
// に広げ、最後に S の大きさを出す。オートマトンも区間の性質も使わないので、提出とは別の考え方になる。
// O((B - A + 1) |S|) なので、B - A が小さいときだけ使う。
#include <cstdio>
#include <unordered_set>
#include <vector>

using u64 = unsigned long long;

int main() {
  u64 a, b;
  if (scanf("%llu %llu", &a, &b) != 2) return 1;
  std::vector<u64> values;  // S の要素
  std::unordered_set<u64> seen;
  auto add = [&](u64 v) {
    if (seen.insert(v).second) values.push_back(v);
  };
  for (u64 x = a;; ++x) {
    size_t before = values.size();  // x と OR を取るのは、x を足す前からある要素だけでよい
    add(x);
    for (size_t i = 0; i < before; ++i) add(values[i] | x);
    if (x == b) break;
  }
  printf("%zu\n", values.size());
}
