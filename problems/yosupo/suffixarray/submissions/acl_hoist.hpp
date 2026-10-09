#pragma once
// acl_char の SA-IS で、induced sorting のラムダが捕まえた値 (n、sa、ls、s の先頭) をローカルに写してから使う版。ACL はラムダが参照で
// 捕まえるので、x86_64 の clang では、sa への書き込みのたびに n と ls の先頭を読み直していた (1 要素で 3 回)。Library の SuffixArray
// との 1.5 倍の差がここから来ているかを見る。ほかは acl_char と同じ。以下は acl_char の説明。
// acl の SA-IS で、L 型と S 型を vector<bool> でなく vector<char> に持つ版。induced sorting では、接尾辞配列を順に読んだあと、
// ls[v - 1] を散らばった位置から読む。vector<bool> だと 1 bit を取り出す手間が毎回かかるので、Library の SuffixArray (vector<char>)
// との 1.5 倍の差がここから来ているかを見る。ほかは acl と同じ。以下は acl の説明。
// AtCoder Library (https://github.com/atcoder/ac-library、CC0 1.0) の atcoder/string.hpp から、suffix_array (文字列版) と、それが使う
// sa_naive、sa_doubling、sa_is を写したもの。中身は変えず、名前空間だけ acl_sa に移した。SA-IS は L 型と S 型を vector<bool> に持ち、
// バケットの境界を L 型と S 型で分けて持つ。10 文字未満は素朴な比較、40 文字未満は prefix doubling (std::sort) に切り替え、再帰の
// 途中で短くなったときも同じにする。
// 接尾辞配列の実装を比べた記録は algo-notes の notes/suffix-array.md にある。
#include <algorithm>
#include <cassert>
#include <numeric>
#include <string>
#include <vector>
#include "pj.hpp"

namespace acl_hoist_sa {

namespace internal {

inline std::vector<int> sa_naive(const std::vector<int>& s) {
    int n = int(s.size());
    std::vector<int> sa(n);
    std::iota(sa.begin(), sa.end(), 0);
    std::sort(sa.begin(), sa.end(), [&](int l, int r) {
        if (l == r) return false;
        while (l < n && r < n) {
            if (s[l] != s[r]) return s[l] < s[r];
            l++;
            r++;
        }
        return l == n;
    });
    return sa;
}

inline std::vector<int> sa_doubling(const std::vector<int>& s) {
    int n = int(s.size());
    std::vector<int> sa(n), rnk = s, tmp(n);
    std::iota(sa.begin(), sa.end(), 0);
    for (int k = 1; k < n; k *= 2) {
        auto cmp = [&](int x, int y) {
            if (rnk[x] != rnk[y]) return rnk[x] < rnk[y];
            int rx = x + k < n ? rnk[x + k] : -1;
            int ry = y + k < n ? rnk[y + k] : -1;
            return rx < ry;
        };
        std::sort(sa.begin(), sa.end(), cmp);
        tmp[sa[0]] = 0;
        for (int i = 1; i < n; i++) {
            tmp[sa[i]] = tmp[sa[i - 1]] + (cmp(sa[i - 1], sa[i]) ? 1 : 0);
        }
        std::swap(tmp, rnk);
    }
    return sa;
}

// SA-IS, linear-time suffix array construction
// Reference:
// G. Nong, S. Zhang, and W. H. Chan,
// Two Efficient Algorithms for Linear Time Suffix Array Construction
template <int THRESHOLD_NAIVE = 10, int THRESHOLD_DOUBLING = 40>
std::vector<int> sa_is(const std::vector<int>& s, int upper) {
    int n = int(s.size());
    if (n == 0) return {};
    if (n == 1) return {0};
    if (n == 2) {
        if (s[0] < s[1]) {
            return {0, 1};
        } else {
            return {1, 0};
        }
    }
    if (n < THRESHOLD_NAIVE) {
        return sa_naive(s);
    }
    if (n < THRESHOLD_DOUBLING) {
        return sa_doubling(s);
    }

    std::vector<int> sa(n);
    std::vector<char> ls(n);
    for (int i = n - 2; i >= 0; i--) {
        ls[i] = (s[i] == s[i + 1]) ? ls[i + 1] : (s[i] < s[i + 1]);
    }
    std::vector<int> sum_l(upper + 1), sum_s(upper + 1);
    for (int i = 0; i < n; i++) {
        if (!ls[i]) {
            sum_s[s[i]]++;
        } else {
            sum_l[s[i] + 1]++;
        }
    }
    for (int i = 0; i <= upper; i++) {
        sum_s[i] += sum_l[i];
        if (i < upper) sum_l[i + 1] += sum_s[i];
    }

    auto induce = [&](const std::vector<int>& lms) {
        // 捕まえた値をローカルに写してから使う (acl_hoist で変えたところ)。参照のまま読むと、sa への書き込みのたびに n や配列の先頭を
        // 読み直す。
        const int N = n;
        int* SA = sa.data();
        const char* LS = ls.data();
        const int* S = s.data();
        std::fill(SA, SA + N, -1);
        std::vector<int> buf(upper + 1);
        int* B = buf.data();
        std::copy(sum_s.begin(), sum_s.end(), buf.begin());
        for (auto d : lms) {
            if (d == N) continue;
            SA[B[S[d]]++] = d;
        }
        std::copy(sum_l.begin(), sum_l.end(), buf.begin());
        SA[B[S[N - 1]]++] = N - 1;
        for (int i = 0; i < N; i++) {
            int v = SA[i];
            if (v >= 1 && !LS[v - 1]) {
                SA[B[S[v - 1]]++] = v - 1;
            }
        }
        std::copy(sum_l.begin(), sum_l.end(), buf.begin());
        for (int i = N - 1; i >= 0; i--) {
            int v = SA[i];
            if (v >= 1 && LS[v - 1]) {
                SA[--B[S[v - 1] + 1]] = v - 1;
            }
        }
    };

    std::vector<int> lms_map(n + 1, -1);
    int m = 0;
    for (int i = 1; i < n; i++) {
        if (!ls[i - 1] && ls[i]) {
            lms_map[i] = m++;
        }
    }
    std::vector<int> lms;
    lms.reserve(m);
    for (int i = 1; i < n; i++) {
        if (!ls[i - 1] && ls[i]) {
            lms.push_back(i);
        }
    }

    induce(lms);

    if (m) {
        std::vector<int> sorted_lms;
        sorted_lms.reserve(m);
        for (int v : sa) {
            if (lms_map[v] != -1) sorted_lms.push_back(v);
        }
        std::vector<int> rec_s(m);
        int rec_upper = 0;
        rec_s[lms_map[sorted_lms[0]]] = 0;
        for (int i = 1; i < m; i++) {
            int l = sorted_lms[i - 1], r = sorted_lms[i];
            int end_l = (lms_map[l] + 1 < m) ? lms[lms_map[l] + 1] : n;
            int end_r = (lms_map[r] + 1 < m) ? lms[lms_map[r] + 1] : n;
            bool same = true;
            if (end_l - l != end_r - r) {
                same = false;
            } else {
                while (l < end_l) {
                    if (s[l] != s[r]) {
                        break;
                    }
                    l++;
                    r++;
                }
                if (l == n || s[l] != s[r]) same = false;
            }
            if (!same) rec_upper++;
            rec_s[lms_map[sorted_lms[i]]] = rec_upper;
        }

        auto rec_sa =
            sa_is<THRESHOLD_NAIVE, THRESHOLD_DOUBLING>(rec_s, rec_upper);

        for (int i = 0; i < m; i++) {
            sorted_lms[i] = lms[rec_sa[i]];
        }
        induce(sorted_lms);
    }
    return sa;
}

}  // namespace internal

inline std::vector<int> suffix_array(const std::string& s) {
    int n = int(s.size());
    std::vector<int> s2(n);
    for (int i = 0; i < n; i++) {
        s2[i] = s[i];
    }
    return internal::sa_is(s2, 255);
}

}  // namespace acl_hoist_sa

struct Solver {
  string s;
  vector<int> sa;

  explicit Solver(const string &s) : s(s) {}

  void run() { sa = acl_hoist_sa::suffix_array(s); }

  const vector<int> &answer() const { return sa; }
};
