// TUPC 2022 K (Lebesgue Integral) https://atcoder.jp/contests/tupc2022/tasks/tupc2022_k
//
// A をソートし、連続する区間に分けて Σ (区間の最大) × (区間の長さ) を最小にする。K は N まで大きいので
// O(KN) は間に合わない。1 区間あたり λ を足した DP を解き、区間数が K 以上になる最大の λ を二分探索する
// (Alien DP)。内側の DP は傾き -j、問い A_i がどちらも単調なので両端キューの CHT で O(N)。
// 期待出力を作る側なので、ライブラリを include しない。
#include <algorithm>
#include <cstdint>
#include <iostream>
#include <vector>
using namespace std;
using i64= long long;
signed main() {
 cin.tie(0);
 ios::sync_with_stdio(false);
 int N, K;
 if(!(cin >> N >> K)) return 0;
 vector<i64> A(N);
 for(auto& x: A) cin >> x;
 sort(A.begin(), A.end());
 const int Kp= min(K, N);
 // 値を value * BASE + (N - 区間数) と詰める。小さいほど良く、同点は区間数が多いほうが勝つ。
 // 値は N * maxA + λ * N ≤ 2e10、BASE = N + 1 ≤ 1e5 + 1 なので 64 bit に収まる。
 const i64 BASE= N + 1;
 // λ を足した DP。戻り値は詰めた値。
 auto solve= [&](i64 lam) -> i64 {
  vector<i64> a, b;  // 包絡に残っている直線 y = a x + b (傾きは狭義減少)
  a.reserve(N + 1), b.reserve(N + 1);
  size_t head= 0;  // 問いが増えるので、これより前は見ない
  auto val= [&](size_t k, i64 x) { return a[k] * x + b[k]; };
  auto add= [&](i64 na, i64 nb) {
   // 末尾の直線 l2 が包絡に出ないか。l1 の傾き > l2 > l3 (= 新しい直線) のとき、
   // l1 と l2 の交点が l2 と l3 の交点より右なら l2 は要らない。
   while(a.size() >= head + 2) {
    const size_t k= a.size() - 1;
    const __int128 lhs= (__int128)(b[k] - b[k - 1]) * (a[k] - na);
    const __int128 rhs= (__int128)(nb - b[k]) * (a[k - 1] - a[k]);
    if(lhs >= rhs) a.pop_back(), b.pop_back();
    else break;
   }
   a.push_back(na), b.push_back(nb);
  };
  // j = 0: 値 0、区間数 0。
  i64 g= N;
  add(0, g);
  for(int i= 1; i <= N; ++i) {
   const i64 x= A[i - 1];
   while(head + 1 < a.size() && val(head + 1, x) <= val(head, x)) ++head;
   // 最後の区間を (j, i] にすると、値が A_i (i - j) + λ、区間数が 1 増える。
   g= val(head, x) + (i64)i * x * BASE + lam * BASE - 1;
   add(-(i64)i * BASE, g);
  }
  return g;
 };
 // λ = 0 では区間数 N、λ ≥ maxA * N + 1 では 1 なので、区間数 Kp 以上になる最大の λ を探す。
 i64 lo= 0, hi= A.back() * N + 1;
 while(lo + 1 < hi) {
  const i64 mid= (lo + hi) / 2, enc= solve(mid);
  if(N - (enc % BASE) >= Kp) lo= mid;
  else hi= mid;
 }
 const i64 enc= solve(lo);
 cout << enc / BASE - lo * Kp << '\n';
 return 0;
}
