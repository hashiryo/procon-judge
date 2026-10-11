// harness: 各提出が定義する run(u, n, v, m, w) をケースごとに呼んで、T ケース分をまとめて計測する。
// run は U = span(u[0..n)) と V = span(v[0..m)) の共通部分の基底を w に書き、その本数を返す (w は 30 本分ある)。
// yosupo "Intersection of F_2 vector spaces" 形式の I/O。
#include "pj.hpp"
#include "common.hpp"

#ifndef SUBMISSION_HPP
#define SUBMISSION_HPP "submissions/naive.hpp"
#endif
#include SUBMISSION_HPP

signed main() {
    int T;
    must_scan(scanf("%d", &T), 1);
    vector<int> nu(T), nv(T);
    vector<size_t> ou(T + 1), ov(T + 1);
    vector<u32> us, vs;
    us.reserve((size_t) T * 30), vs.reserve((size_t) T * 30);
    for (int t = 0; t < T; ++t) {
        must_scan(scanf("%d", &nu[t]), 1);
        for (int i = 0; i < nu[t]; ++i) {
            u32 x;
            must_scan(scanf("%u", &x), 1);
            us.push_back(x);
        }
        must_scan(scanf("%d", &nv[t]), 1);
        for (int i = 0; i < nv[t]; ++i) {
            u32 x;
            must_scan(scanf("%u", &x), 1);
            vs.push_back(x);
        }
        ou[t + 1] = us.size(), ov[t + 1] = vs.size();
    }
    vector<u32> ws((size_t) T * 30);
    vector<int> ks(T);

    auto t0 = chrono::steady_clock::now();
    for (int t = 0; t < T; ++t) ks[t] = run(us.data() + ou[t], nu[t], vs.data() + ov[t], nv[t], ws.data() + (size_t) t * 30);
    auto t1 = chrono::steady_clock::now();

    string out;
    out.reserve((size_t) T * 64);
    char buf[16];
    for (int t = 0; t < T; ++t) {
        auto [p, ec] = to_chars(buf, buf + 16, ks[t]);
        out.append(buf, p);
        for (int i = 0; i < ks[t]; ++i) {
            out += ' ';
            auto [q, ec2] = to_chars(buf, buf + 16, ws[(size_t) t * 30 + i]);
            out.append(buf, q);
        }
        out += '\n';
    }
    fputs(out.c_str(), stdout);
    report_metrics((long long)chrono::duration_cast<chrono::nanoseconds>(t1 - t0).count());
    return 0;
}
