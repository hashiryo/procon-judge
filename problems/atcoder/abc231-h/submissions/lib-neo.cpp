// 重み付き (分割 + 分割)。取り除く駒の集合を、行ごと列ごとに (次数 - 1) 個までの分割マトロイド 2 つの共通独立集合にして、
// 重みの和の最大を NeoLibrary の weighted_matroid_intersection で求める。
#include <iostream>
#include <vector>
#include <numeric>
#include <algorithm>
#include "neo/algo/optimize/matroid/matroid_intersection.hpp"
#include "neo/algo/optimize/matroid/matroids.hpp"
using namespace std;
int main() {
 cin.tie(0);
 ios::sync_with_stdio(false);
 int H, W, N;
 cin >> H >> W >> N;
 vector<long long> C(N);
 vector<int> p1(N), p2(N), sz1(H, -1), sz2(W, -1);
 for(int i= 0; i < N; i++) {
  int A, B;
  cin >> A >> B >> C[i], A--, B--;
  p1[i]= A, p2[i]= B, sz1[A]++, sz2[B]++;
 }
 auto [best, I]= weighted_matroid_intersection(PartitionMatroid(p1, sz1), PartitionMatroid(p2, sz2), C);
 cout << accumulate(C.begin(), C.end(), 0ll) - max(0ll, *max_element(best.begin(), best.end())) << '\n';
 return 0;
}
