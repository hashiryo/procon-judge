// s8pc-1-h の愚直解。食品ごとに square1001、E869120、うさぎに渡すか残すか (残すのは E 個まで) を
// 全部試し、square1001 との和の差が 2 人とも D 以下になる分け方を数える。半分ずつ列挙して 2 次元の
// 範囲で数えることをしないので、KDTree や SegmentTree_2D とは別の考え方になる。N が小さいときだけ使う。
#include <cstdio>
#include <vector>

int n, e;
long long d;
std::vector<long long> a;

// i 番目より前の食品は配り終えていて、3 人の和が s[0], s[1], s[2]、残した数が left。
long long count(int i, long long s0, long long s1, long long s2, int left) {
  if (i == n) {
    long long x = s0 - s1, y = s0 - s2;
    return -d <= x && x <= d && -d <= y && y <= d;
  }
  long long ways = count(i + 1, s0 + a[i], s1, s2, left) + count(i + 1, s0, s1 + a[i], s2, left) +
                   count(i + 1, s0, s1, s2 + a[i], left);
  if (left < e) ways += count(i + 1, s0, s1, s2, left + 1);
  return ways;
}

int main() {
  if (scanf("%d %lld %d", &n, &d, &e) != 3) return 1;
  a.resize(n);
  for (auto& x : a)
    if (scanf("%lld", &x) != 1) return 1;
  printf("%lld\n", count(0, 0, 0, 0, 0));
}
