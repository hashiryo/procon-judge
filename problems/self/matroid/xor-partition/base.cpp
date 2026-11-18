// 自作の問題。GF(2) の線形マトロイド (64 bit のベクトルの xor での線形独立) と分割マトロイドの最大共通独立集合を、
// 性質の違う族で測る。ベクトルに色があり、色 c のベクトルは cap[c] 本まで使える。線形独立なベクトルの集合のうち、
// 要素数が最大のものを求める (Pick Your Own Nim の形を広げたもの)。
//
// 提出は次を実装する。
//   struct Solver {
//     Solver(const vector<u64> &vec, const vector<int> &color, const vector<int> &cap);
//     void run();                  // 最大共通独立集合を求める
//     vector<int> answer() const;  // 選んだベクトルの番号
//   };
// color[i] はベクトル i の色で 0 以上 cap.size() 未満、cap[c] >= 0。0 のベクトルもありうる。
//
// 入力は 1 行で、族の名前、seed、引数を並べる。ベクトルはハーネスが計測の前に作り、順を乱択で並べ替える。構築と run を
// 測る。answer は計測区間の外で呼び、ハーネスが番号の範囲と重複、線形独立であること、色ごとの本数を確かめてから、
// 要素数を出す (崩れていれば invalid)。引数の参照は計測区間のあとまで生きているので、提出は参照を持ってよい。
//
//   random M D C K     下位 D bit が乱択のベクトルを M 本。色は C 色から一様に選び、どの色も上限 K
//   low M D R C K      D bit の乱択の R 本が張る空間から乱択に選んだベクトルを M 本 (階数は R 以下)。色は C 色、上限 K
//   nim M D            D 色に M / D 本ずつ分け、どの色も上限 1 (Pick Your Own Nim の形)。下位 D bit が乱択
//   sparse M D W C K   D bit のうち乱択の W bit を立てたベクトルを M 本。色は C 色、上限 K
//   chain M D C K      e_i xor e_{i+1} の形 (D bit の中で i を乱択) のベクトルを M 本。色は C 色、上限 K。グラフ的
//                      マトロイドの道と同じ形
//   match M D          D 色 (上限 1) と D 個の単位ベクトル。色 c のベクトルは e_c か e_{c+1 mod D} で、色を一様に選ぶ。
//                      色と単位ベクトルの二部マッチングと同じ形で、先に取ったものを入れ替える長い増加路が要る
#include "pj.hpp"
#include <algorithm>
#include <numeric>
#include <string>

#ifndef SUBMISSION_HPP
#define SUBMISSION_HPP "reference.hpp"
#endif
#include SUBMISSION_HPP

namespace gen {
struct Rng {
  u64 x;
  u64 next() {
    u64 z = (x += 0x9E3779B97F4A7C15ULL);
    z = (z ^ (z >> 30)) * 0xBF58476D1CE4E5B9ULL;
    z = (z ^ (z >> 27)) * 0x94D049BB133111EBULL;
    return z ^ (z >> 31);
  }
  // [0, n)
  int below(u64 n) { return (int)((unsigned __int128)next() * n >> 64); }
  u64 bits(int d) { return d >= 64 ? next() : next() & ((1ULL << d) - 1); }
};

struct Instance {
  vector<u64> v;
  vector<int> color, cap;
};

void uniform_colors(Rng &rng, Instance &g, int C, int K) {
  g.color.resize(g.v.size());
  for (auto &c : g.color) c = rng.below(C);
  g.cap.assign(C, K);
}

Instance random(Rng &rng, int M, int D, int C, int K) {
  Instance g;
  g.v.resize(M);
  for (auto &x : g.v) x = rng.bits(D);
  uniform_colors(rng, g, C, K);
  return g;
}

Instance low(Rng &rng, int M, int D, int R, int C, int K) {
  Instance g;
  vector<u64> base(R);
  for (auto &b : base) b = rng.bits(D);
  g.v.resize(M);
  for (auto &x : g.v) {
    const u64 s = rng.bits(R);
    x = 0;
    for (int k = 0; k < R; ++k)
      if (s >> k & 1) x ^= base[k];
  }
  uniform_colors(rng, g, C, K);
  return g;
}

Instance nim(Rng &rng, int M, int D) {
  Instance g;
  g.v.resize(M);
  g.color.resize(M);
  for (int i = 0; i < M; ++i) g.v[i] = rng.bits(D), g.color[i] = i % D;
  g.cap.assign(D, 1);
  return g;
}

Instance sparse(Rng &rng, int M, int D, int W, int C, int K) {
  Instance g;
  g.v.resize(M);
  for (auto &x : g.v) {
    x = 0;
    for (int k = 0; k < W; ++k) x |= 1ULL << rng.below(D);
  }
  uniform_colors(rng, g, C, K);
  return g;
}

Instance chain(Rng &rng, int M, int D, int C, int K) {
  Instance g;
  g.v.resize(M);
  for (auto &x : g.v) {
    const int i = rng.below(D - 1);
    x = (1ULL << i) | (1ULL << (i + 1));
  }
  uniform_colors(rng, g, C, K);
  return g;
}
Instance match(Rng &rng, int M, int D) {
  Instance g;
  g.v.resize(M);
  g.color.resize(M);
  for (int i = 0; i < M; ++i) {
    const int c = rng.below(D);
    const int b = rng.next() >> 63 ? (c + 1) % D : c;
    g.v[i] = 1ULL << b, g.color[i] = c;
  }
  g.cap.assign(D, 1);
  return g;
}
}  // namespace gen

signed main() {
  const string fam = read_token();
  long long seed;
  must_scan(scanf("%lld", &seed), 1);
  gen::Rng rng{(u64)seed};
  auto args = [&](int k) {
    vector<int> a(k);
    for (auto &x : a) must_scan(scanf("%d", &x), 1);
    return a;
  };
  gen::Instance g;
  if (fam == "random") { auto a = args(4); g = gen::random(rng, a[0], a[1], a[2], a[3]); }
  else if (fam == "low") { auto a = args(5); g = gen::low(rng, a[0], a[1], a[2], a[3], a[4]); }
  else if (fam == "nim") { auto a = args(2); g = gen::nim(rng, a[0], a[1]); }
  else if (fam == "sparse") { auto a = args(5); g = gen::sparse(rng, a[0], a[1], a[2], a[3], a[4]); }
  else if (fam == "chain") { auto a = args(4); g = gen::chain(rng, a[0], a[1], a[2], a[3]); }
  else if (fam == "match") { auto a = args(2); g = gen::match(rng, a[0], a[1]); }
  else {
    fprintf(stderr, "unknown family %s\n", fam.c_str());
    return 1;
  }
  for (size_t i = g.v.size(); i > 1; --i) {
    const int j = rng.below(i);
    swap(g.v[i - 1], g.v[j]), swap(g.color[i - 1], g.color[j]);
  }

  auto t0 = chrono::steady_clock::now();
  Solver sol(g.v, g.color, g.cap);
  sol.run();
  auto t1 = chrono::steady_clock::now();

  const vector<int> ans = sol.answer();
  const int m = g.v.size();
  vector<char> used(m);
  vector<int> cnt(g.cap.size());
  u64 basis[64] = {};
  string bad;
  for (int i : ans) {
    if (i < 0 || i >= m) { bad = "id out of range"; break; }
    if (used[i]) { bad = "id used twice"; break; }
    used[i] = 1;
    u64 x = g.v[i];
    while (x && basis[63 - __builtin_clzll(x)]) x ^= basis[63 - __builtin_clzll(x)];
    if (!x) { bad = "linearly dependent"; break; }
    basis[63 - __builtin_clzll(x)] = x;
    if (++cnt[g.color[i]] > g.cap[g.color[i]]) { bad = "color over capacity"; break; }
  }
  if (bad.empty()) printf("%d\n", (int)ans.size());
  else printf("invalid: %s\n", bad.c_str());

  report_metrics((long long)chrono::duration_cast<chrono::nanoseconds>(t1 - t0).count());
}
