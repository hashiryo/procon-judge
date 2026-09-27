// abc135-d の愚直解。? に入る数字の組 (? が q 個なら 10^q 通り) を全部試し、できた整数そのものを
// 13 で割って、余りが 5 になるものを数える。桁ごとの DP もオートマトンの積も使わないので、
// 提出とは別の考え方になる。長さ 18 まで (unsigned long long に収まる) の、? の少ない S でだけ使う。
#include <cstdio>
#include <cstring>
#include <vector>

int main() {
  char s[32];
  if (scanf("%31s", s) != 1) return 1;
  int n = strlen(s);
  if (n > 18) return 1;
  std::vector<int> q;
  for (int i = 0; i < n; ++i)
    if (s[i] == '?') q.push_back(i);
  long long total = 1;
  for (size_t i = 0; i < q.size(); ++i) total *= 10;
  long long count = 0;
  for (long long code = 0; code < total; ++code) {
    char t[32];
    memcpy(t, s, n + 1);
    long long c = code;
    for (int i : q) t[i] = '0' + c % 10, c /= 10;
    unsigned long long value = 0;
    for (int i = 0; i < n; ++i) value = value * 10 + (t[i] - '0');
    if (value % 13 == 5) ++count;
  }
  printf("%lld\n", count % 1000000007);
}
