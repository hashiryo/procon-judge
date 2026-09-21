// harness: 提出が定義する struct Solver (unite / same) を使って計測する。
// Library の UnionFind と名前がぶつかるので、提出の型は Solver にしてある。
#include "pj.hpp"
#include "common.hpp"

// CI では -DSUBMISSION_HPP で上書きされる。
// ここでのデフォルトは IDE で base.cpp 単独表示時に補完を効かせるため。
#ifndef SUBMISSION_HPP
#define SUBMISSION_HPP "submissions/naive.hpp"
#endif
#include SUBMISSION_HPP

signed main() {
    int n, q;
    if (scanf("%d %d", &n, &q) != 2) return 1;
    vector<array<int, 3>> qs(q);
    for (auto& e : qs) scanf("%d %d %d", &e[0], &e[1], &e[2]);

    // クエリ列を先読みしてあるので、計測対象は純粋な UnionFind 演算のみ。
    Solver uf(n);
    string out;
    out.reserve(size_t(q) * 2);
    auto t0 = chrono::steady_clock::now();
    for (auto& e : qs) {
        if (e[0] == 0) uf.unite(e[1], e[2]);
        else out += uf.same(e[1], e[2]) ? "1\n" : "0\n";
    }
    auto t1 = chrono::steady_clock::now();
    auto ns = (uint64_t)chrono::duration_cast<chrono::nanoseconds>(t1 - t0).count();

    fputs(out.c_str(), stdout);
    report_metrics((long long)ns);
    return 0;
}
