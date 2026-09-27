// abc336-g の愚直解。列を左から 1 文字ずつ決めて数える。状態は直近の 3 文字と、16 種類の窓それぞれの
// 残りの個数で、残りの個数ごとにメモする。BEST 定理も行列木定理も使わないので、提出とは別の考え方になる。
// 状態の数が残りの個数の組み合わせの数 (各 X + 1 の積) になるので、小さい入力でだけ使う。
#include <cstdio>
#include <unordered_map>

using u64 = unsigned long long;
constexpr u64 P = 998244353;

int x[16];
u64 place[16];  // 残りの個数を混合基数で 1 つの整数にするときの桁の重み
std::unordered_map<u64, u64> memo;

// 直近の 3 文字が last で、残りの個数が code のとき、列の残りの決め方の数。
u64 count(int last, u64 code, int remaining) {
  if (remaining == 0) return 1;
  u64 key = code * 8 + last;
  if (auto it = memo.find(key); it != memo.end()) return it->second;
  u64 total = 0;
  for (int bit = 0; bit < 2; ++bit) {
    int w = last * 2 + bit;  // 次の窓
    if (code / place[w] % (x[w] + 1) == 0) continue;
    total += count(w & 7, code - place[w], remaining - 1);
  }
  return memo[key] = total % P;
}

int main() {
  int n = 0;
  for (int i = 0; i < 16; ++i) {
    if (scanf("%d", &x[i]) != 1) return 1;
    n += x[i];
  }
  u64 code = 0, weight = 1;
  for (int i = 0; i < 16; ++i) {
    place[i] = weight, code += weight * x[i];
    if (weight > (1ULL << 58) / (x[i] + 1)) return 2;  // 組み合わせが多すぎて、この愚直解では解けない
    weight *= x[i] + 1;
  }
  u64 ans = 0;
  for (int first = 0; first < 8; ++first) ans += count(first, code, n);
  printf("%llu\n", ans % P);
}
