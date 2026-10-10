// harness: 各提出が定義する run(u32 a, u32 b) を計測する。b >= 1 で、(g, x) を std::pair<u32, u32> で返す。
// g = gcd(a, b)、a x ≡ g (mod b)、0 <= x < b / g。ACL の internal::inv_gcd と同じ約束で、この (g, x) は 1 通りに決まる。
// 入力は N amode bmode gmode seed の 1 行で、組 (a_i, b_i) は計測の前に _pairs.hpp が 32 bit の幅で作る。
// 計測区間は N 回の呼び出しだけで、呼び出しどうしは独立 (前の答えを次の入力に使わない)。
#include "pj.hpp"
#include "_shared/modulo-test/_common.hpp"
#include "_shared/gcd-test/_pairs.hpp"

#ifndef SUBMISSION_HPP
#define SUBMISSION_HPP "submissions/naive_euclid.hpp"
#endif
#include SUBMISSION_HPP

signed main() {
    cin.tie(0);
    ios::sync_with_stdio(false);
    u64 n, seed;
    string am, bm, gm;
    cin >> n >> am >> bm >> gm >> seed;
    vector<u32> as(n), bs(n);
    {
        vector<u64> a64, b64;
        gcd_test::make_pairs(n, am, bm, gm, seed, a64, b64, true, 32);
        for (u64 i = 0; i < n; ++i) as[i] = u32(a64[i]), bs[i] = u32(b64[i]);
    }

    u64 acc = 0;
    auto t0 = chrono::steady_clock::now();
    for (u64 i = 0; i < n; ++i) {
        auto [g, x] = run(as[i], bs[i]);
        acc = (acc ^ g) * 0x9e3779b97f4a7c15ull;
        acc = (acc ^ x) * 0xff51afd7ed558ccdull;
    }
    auto t1 = chrono::steady_clock::now();

    cout << acc << '\n';
    report_metrics((long long)chrono::duration_cast<chrono::nanoseconds>(t1 - t0).count());
    return 0;
}
