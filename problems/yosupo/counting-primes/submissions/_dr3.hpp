#pragma once
// Deléglise–Rivat の素数の個数 π(x) (3 回目)。_dr2.hpp の葉の切り方に、Gourdon と同じく y とは別の
// 境目 w (y ≤ w) を入れた。節 μ(n) φ(x / n, b) を n ≤ w なら分けるので、
//   S1 = Σ_{n ≤ w, n の素因数は (p_c, y] にあり平方因子なし} μ(n) φ(x / n, c)
//   S2 = Σ_{c < b < a} Σ_{w / p_b < m ≤ w, m の素因数は (p_b, y]} -μ(m) φ(x / (p_b m), b - 1)
// になる。葉の t は x / w 未満なので、hard leaves を数える篩は [1, x / w] で済む。x / w から x / y までは
// P2 のための素数の篩だけを回す。素数の m は y 以下、合成数の m は w 以下。
//
// 以下は 2 回目の説明のまま。_dr.hpp から変えたのは次の 3 つ。
// - c を 6 から 8 にした。17 と 19 の葉は ordinary leaves に移り、φ(t, 8) は 2310 周期の表を 8 回引く。
//   17 と 19 は区間ごとに数えずに消し、残りの個数を popcount でまとめて数える。
// - easy leaves の反転で足す r のうち sparse の範囲に入るものは、sparse の f(q) を 2 回足して
//   引き直さない (primecount 8.7 の Reflected と同じ考え)。
// - 倍数を消す処理を、m が 30 進む 1 周ぶんの 8 個を並べて書いた (類ごとにビットの位置が定数になる)。
//
// y = α x^{1/3}、z = x / y、a = π(y)、c = 8 (p_c = 19) として
//   π(x) = S1 + S2 + a - 1 - P2
//   S1 = Σ_{n ≤ y, n は平方因子なし, lpf(n) > p_c} μ(n) φ(x / n, c)            (ordinary leaves)
//   S2 = Σ_{c < b < a} Σ_{y / p_b < m ≤ y, lpf(m) > p_b} -μ(m) φ(x / (p_b m), b - 1)  (special leaves)
//   P2 = Σ_{y < p_b ≤ √x} (π(x / p_b) - b + 1)
// を求める。special leaves は t = x / (p_b m) で分ける。
//   t ≥ p_b^2 (hard):       [1, z] を区間ごとに篩い、p_1..p_{b-1} を除いた残りを数える。
//   p_b ≤ t < p_b^2 (easy):  φ(t, b - 1) = π(t) - b + 2。π の表を引く。
//   t < p_b (trivial):       φ = 1。素数 q の個数だけで足す。
// m が素数 q の easy leaves のうち q > √(x / p) の部分は、Gourdon の反転
//   Σ_{α < q ≤ β} π(N / q) = π(β) π(N / β) - π(α) π(N / α) + Σ_{N / β < r ≤ N / α} π(N / r)
// で r ≤ √N の和に直す (π(N / q) が q について階段状になる部分を 1 つずつ数えない)。
//
// 篩と π の表は 30 の車輪で持つ (1 バイトに 30k + {1, 7, 11, 13, 17, 19, 23, 29} の 8 個)。
// π の表は hard leaves と同じ篩の走査で作る (区間の篩を √z まで進めた残りが素数)。
// 割り算 N / d は、N d < 2^64 なら floor(2^64 / d) + 1 との掛け算の上位で正確に求まる。
#include "../common.hpp"

namespace dr3 {

#ifdef DR_PROF
#define DR_MARK(n) auto n = std::chrono::steady_clock::now()
#define DR_MS(a, b) std::chrono::duration<double, std::milli>(b - a).count()
#else
#define DR_MARK(n)
#endif

inline u64 isqrt(u64 n) {
    u64 r = (u64)std::sqrt((double)n);
    while (r * r > n) --r;
    while ((r + 1) * (r + 1) <= n) ++r;
    return r;
}
inline u64 icbrt(u64 n) {
    u64 r = (u64)std::cbrt((double)n);
    while (r * r * r > n) --r;
    while ((r + 1) * (r + 1) * (r + 1) <= n) ++r;
    return r;
}
inline u64 mulhi(u64 a, u64 b) { return (u64)(((u128)a * b) >> 64); }
// N d < 2^64 のとき mulhi(N, magic(d)) = N / d
inline u64 magic(u64 d) { return ~u64(0) / d + 1; }
// n < 2^53 なら double の割り算で。正しく丸めた商は floor(n / d) か 1 大きいだけなので、1 回直せばよい。
inline u64 fdiv(u64 n, u64 d) {
    if (n < (u64(1) << 53)) {
        const u64 q = (u64)((double)n / (double)d);
        return q - (q * d > n);
    }
    return n / d;
}

inline constexpr u32 WR[8] = {1, 7, 11, 13, 17, 19, 23, 29};
inline constexpr std::array<u8, 30> BI = [] {
    std::array<u8, 30> t{};
    for (auto& v : t) v = 255;
    for (u32 i = 0; i < 8; ++i) t[WR[i]] = (u8)i;
    return t;
}();
// MASK[j]: 240 個の塊の中で j 以下の数のビット
inline constexpr std::array<u64, 240> MASK = [] {
    std::array<u64, 240> t{};
    for (u32 j = 0; j < 240; ++j) {
        u64 m = 0;
        for (u32 k = 0; k < 8; ++k)
            for (u32 i = 0; i < 8; ++i)
                if (30 * k + WR[i] <= j) m |= u64(1) << (8 * k + i);
        t[j] = m;
    }
    return t;
}();
// 素数 p ≡ WR[cls] (mod 30) の倍数 p m (m ≡ WR[i]) のビットの位置
inline constexpr std::array<std::array<u8, 8>, 8> WBIT = [] {
    std::array<std::array<u8, 8>, 8> t{};
    for (u32 c = 0; c < 8; ++c)
        for (u32 i = 0; i < 8; ++i) t[c][i] = BI[(WR[c] * WR[i]) % 30];
    return t;
}();

// 奇数だけの篩で数える。小さい x 用。
inline u64 pi_simple(u64 x) {
    if (x < 2) return 0;
    u64 h = (x - 1) / 2;
    std::vector<char> comp(h + 1, 0);
    u64 cnt = 1;
    for (u64 i = 1; i <= h; ++i) {
        if (comp[i]) continue;
        ++cnt;
        u64 p = 2 * i + 1;
        for (u64 j = (p * p - 1) / 2; j <= h; j += p) comp[j] = 1;
    }
    return cnt;
}

struct PiTable {
    struct E {
        u64 bits;  // 240 w + 30 k + WR[i] が素数なら bit 8 k + i
        u64 cnt;   // 240 w 未満の 7 以上の素数の個数
    };
    std::vector<E> t;
    // n ≥ 5
    u64 operator()(u64 n) const {
        const E& e = t[n / 240];
        return 3 + e.cnt + (u64)std::popcount(e.bits & MASK[n % 240]);
    }
};

// φ(t, 5) を 2310 周期の表で引き、φ(t, k) = φ(t, k - 1) - φ(t / p_k, k - 1) で 13、17、19 を足す。
struct Phi8 {
    uint16_t tab[2310];
    Phi8() {
        u32 c = 0;
        for (u32 i = 0; i < 2310; ++i) {
            if (i % 2 && i % 3 && i % 5 && i % 7 && i % 11) ++c;
            tab[i] = (uint16_t)c;
        }
    }
    u64 phi5(u64 t) const { return t / 2310 * 480 + tab[t % 2310]; }
    u64 phi6(u64 t) const { return phi5(t) - phi5(t / 13); }
    u64 phi7(u64 t) const { return phi6(t) - phi6(t / 17); }
    u64 operator()(u64 t) const { return phi7(t) - phi7(t / 19); }
};

// 篩う素数の状態。pos は今の区間の先頭からのバイトの位置、i は倍数 p m の m の車輪の位置。
struct SievingPrime {
    u32 pos, a;
    u8 cls, i;
};

// m が 30 進む 1 周で倍数は p バイト進む。周の先頭 (m ≡ 1) から k 番目の倍数までのバイトの差は
// (p / 30) (WR[k] - 1) + floor((p mod 30) WR[k] / 30) で、後ろの項は類 cls と k だけで決まる。
inline constexpr std::array<std::array<u8, 8>, 8> WOFF = [] {
    std::array<std::array<u8, 8>, 8> t{};
    for (u32 c = 0; c < 8; ++c)
        for (u32 k = 0; k < 8; ++k) t[c][k] = (u8)(WR[c] * WR[k] / 30);
    return t;
}();

// 区間の [0, nbytes) バイトにある p の倍数を消す。COUNT なら消えた数を cnt から引く。
// 周の途中は 1 つずつ、周が丸ごと入るところは 8 個を並べて消す。
template <u32 CLS, bool COUNT>
inline void cross_cls(u8* seg, u32 nbytes, SievingPrime& s, u64& cnt) {
    const u32 a = s.a, p = 30 * a + WR[CLS];
    const u32 off[8] = {0,
                        a * 6 + WOFF[CLS][1],
                        a * 10 + WOFF[CLS][2],
                        a * 12 + WOFF[CLS][3],
                        a * 16 + WOFF[CLS][4],
                        a * 18 + WOFF[CLS][5],
                        a * 22 + WOFF[CLS][6],
                        a * 28 + WOFF[CLS][7]};
    auto clr = [&](u32 q, u32 bit) {
        if constexpr (COUNT) {
            const u8 v = seg[q];
            cnt -= (v >> bit) & 1;
            seg[q] = (u8)(v & ~(1u << bit));
        } else {
            seg[q] &= (u8)~(1u << bit);
        }
    };
    u32 i = s.i;
    u32 base = s.pos - off[i];  // 周の先頭 (区間より前なら 2^32 で回る)
    for (; i < 8; ++i) {
        const u32 q = base + off[i];
        if (q >= nbytes) {
            s.pos = q - nbytes;
            s.i = (u8)i;
            return;
        }
        clr(q, WBIT[CLS][i]);
    }
    base += p;
    const u32 o1 = off[1], o2 = off[2], o3 = off[3], o4 = off[4], o5 = off[5], o6 = off[6], o7 = off[7];
    while (base + o7 < nbytes) {
        clr(base, WBIT[CLS][0]);
        clr(base + o1, WBIT[CLS][1]);
        clr(base + o2, WBIT[CLS][2]);
        clr(base + o3, WBIT[CLS][3]);
        clr(base + o4, WBIT[CLS][4]);
        clr(base + o5, WBIT[CLS][5]);
        clr(base + o6, WBIT[CLS][6]);
        clr(base + o7, WBIT[CLS][7]);
        base += p;
    }
    for (i = 0;; ++i) {
        const u32 q = base + off[i];
        if (q >= nbytes) {
            s.pos = q - nbytes;
            s.i = (u8)i;
            return;
        }
        clr(q, WBIT[CLS][i]);
    }
}
template <bool COUNT>
inline void cross_off(u8* seg, u32 nbytes, SievingPrime& s, u64& cnt) {
    switch (s.cls) {
        case 0: cross_cls<0, COUNT>(seg, nbytes, s, cnt); break;
        case 1: cross_cls<1, COUNT>(seg, nbytes, s, cnt); break;
        case 2: cross_cls<2, COUNT>(seg, nbytes, s, cnt); break;
        case 3: cross_cls<3, COUNT>(seg, nbytes, s, cnt); break;
        case 4: cross_cls<4, COUNT>(seg, nbytes, s, cnt); break;
        case 5: cross_cls<5, COUNT>(seg, nbytes, s, cnt); break;
        case 6: cross_cls<6, COUNT>(seg, nbytes, s, cnt); break;
        default: cross_cls<7, COUNT>(seg, nbytes, s, cnt); break;
    }
}

// y = alpha x^{1/3}、w = beta y
inline u64 prime_pi(u64 x, double alpha, double beta) {
    if (x < 100000) return pi_simple(x);
    DR_MARK(T0);
    constexpr u32 c = 8;
    const u64 x13 = icbrt(x);
    const u64 sx = isqrt(x);
    u64 y = (u64)(alpha * (double)x13);
    y = std::max(y, x13 + 1);
    y = std::min(y, sx);
    const u64 w = std::min(sx, std::max(y, (u64)(beta * (double)y)));
    const u64 z = x / y;   // P2 のための素数の篩の上限
    const u64 zh = x / w;  // 葉の t の上限
    const u64 sz = isqrt(z);

    // y + 2 までの素数。clustered の反転で引く r は √(x / p) + 2 以下で、
    // 反転する範囲があるのは √(x / p) < y のときだけなので、y + 1 までで足りる。
    const u64 L = y + 2;
    std::vector<u32> primes;
    {
        const u64 h = (L - 1) / 2;  // 3, 5, ..., 2 h + 1
        std::vector<u8> comp(h + 1, 0);
        for (u64 i = 1; (2 * i + 1) * (2 * i + 1) <= L; ++i) {
            if (comp[i]) continue;
            const u64 p = 2 * i + 1;
            for (u64 j = (p * p) / 2; j <= h; j += p) comp[j] = 1;
        }
        primes.resize(h + 2);
        primes[0] = 0;  // 1 始まり
        primes[1] = 2;
        size_t k = 2;
        for (u64 i = 1; i <= h; ++i) {
            primes[k] = (u32)(2 * i + 1);
            k += !comp[i];
        }
        primes.resize(k);
    }
    auto npr_upto = [&](u64 n) -> u64 {  // primes 配列で n 以下の素数の個数
        return (u64)(std::upper_bound(primes.begin() + 1, primes.end(), (u32)n) - primes.begin()) - 1;
    };
    const u64 a = npr_upto(y);
    const u64 nsz = npr_upto(sz);
    // w 以下の平方因子のない合成数で、素因数が (p_c, y] にあるもの (昇順)。素数の積を深さ優先でたどって
    // 長さ w の表に lpf と μ を書き、小さい順に詰める。合成数の lpf は √w 以下なので 15 ビットに入る。
    std::vector<u32> comp_m, comp_lpf;
    std::vector<int8_t> comp_mu;
    {
        std::vector<uint16_t> tag(w + 1, 0);  // lpf | (μ < 0 ? 0x8000 : 0)
        size_t n = 0;
        auto dfs = [&](auto& self, u64 m, size_t i, u32 lp, int32_t mu) -> void {
            for (; i <= a; ++i) {
                const u64 mm = m * primes[i];
                if (mm > w) break;
                if (lp) {
                    tag[mm] = (uint16_t)(lp | (mu > 0 ? 0x8000 : 0));  // mm の μ は -mu
                    ++n;
                }
                self(self, mm, i + 1, lp ? lp : primes[i], -mu);
            }
        };
        dfs(dfs, 1, c + 1, 0, 1);
        comp_m.resize(n + 1);
        size_t k = 0;
        for (u64 m = 1; m <= w; ++m) {
            comp_m[k] = (u32)m;
            k += tag[m] != 0;
        }
        comp_m.resize(n);
        comp_lpf.resize(n);
        comp_mu.resize(n);
        for (size_t i = 0; i < n; ++i) {
            const uint16_t v = tag[comp_m[i]];
            comp_lpf[i] = v & 0x7fff;
            comp_mu[i] = (v & 0x8000) ? -1 : 1;
        }
    }
    std::vector<u64> pmagic(primes.size());
    for (size_t i = 1; i < primes.size(); ++i) pmagic[i] = magic(primes[i]);
    std::vector<u64> cmagic(comp_m.size());
    for (size_t i = 0; i < comp_m.size(); ++i) cmagic[i] = magic(comp_m[i]);
    DR_MARK(T1);

    // hard leaves の範囲。b ごとに m ∈ (max(y/p, p), min(y, x/p^3)]。
    // 素数の m は primes の添字 (pi_lo, pi_cur]、合成数の m は comp_m の添字 [ci_lo, ci_cur] で持ち、
    // どちらも上から下る (t が増える向き)。範囲が空でないなら p^2 ≤ z なので、b ≤ π(√z) に収まる。
    u64 bmax = c;
    std::vector<u64> N(a + 1, 0);
    std::vector<u32> pi_cur(a + 1, 0), pi_lo(a + 1, 0);
    std::vector<int64_t> ci_cur(a + 1, -1), ci_lo(a + 1, 0);
    for (u64 b = c + 1; b < a; ++b) {
        const u64 p = primes[b];
        if (p * p > zh) break;
        const u64 p3 = x / (p * p * p);
        // 素数の m ∈ (max(w/p, p), min(y, x/p^3)]、合成数の m ∈ (max(w/p, p^2), min(w, x/p^3)]
        // (合成数の m の素因数は p より大きいので m > p^2)
        const u64 lop = std::max(w / p, p), hip = std::min(y, p3);
        const u64 loc = std::max(w / p, p * p), hic = std::min(w, p3);
        if (lop >= hip && loc >= hic) continue;
        bmax = b;
        N[b] = x / p;
        pi_lo[b] = (u32)npr_upto(lop);
        pi_cur[b] = (u32)std::max(npr_upto(lop), npr_upto(hip));
        ci_lo[b] = std::upper_bound(comp_m.begin(), comp_m.end(), (u32)loc) - comp_m.begin();
        ci_cur[b] = std::max<int64_t>(ci_lo[b] - 1, (std::upper_bound(comp_m.begin(), comp_m.end(), (u32)std::min(hic, w)) - comp_m.begin()) - 1);
    }

    // 区間の篩。区間の先頭 low は 240 の倍数で、バイト k は 30 (low / 30 + k) + WR[i]。
    constexpr u32 SWORDS = 2048;
    constexpr u64 SEGN = 240 * (u64)SWORDS;  // 区間に入る数の幅
    std::vector<u64> seg64(SWORDS + 1, 0);
    u8* seg = reinterpret_cast<u8*>(seg64.data());
    // 7, 11, 13 の倍数を除く型 (1001 バイト周期)
    std::vector<u8> pat(1001);
    for (u32 k = 0; k < 1001; ++k) {
        u8 v = 0;
        for (u32 i = 0; i < 8; ++i) {
            u64 n = 30 * (u64)k + WR[i];
            if (n % 7 && n % 11 && n % 13) v |= (u8)(1u << i);
        }
        pat[k] = v;
    }
    // 17 以上の素数で篩う。p_c までと hard leaves のある素数は φ のために p 自身から消し、
    // 残りは π の表のためだけなので p^2 から消す。
    const u64 nsp = std::max<u64>(nsz, c);  // √z が p_c より小さくても p_c までは消す
    std::vector<SievingPrime> sp(std::max(nsp, bmax) + 1);
    for (u64 b = 7; b <= nsp; ++b) {
        const u64 p = primes[b];
        const u64 m0 = b <= std::max<u64>(bmax, c) ? 1 : p;
        const u64 n0 = p * m0;
        sp[b] = SievingPrime{(u32)(n0 / 30), (u32)(p / 30), BI[p % 30], BI[m0 % 30]};
    }
    std::vector<i64> phi(bmax + 1, 0);
    std::vector<u32> pre(SWORDS + 1);
    PiTable pt;
    pt.t.resize(z / 240 + 1);
    u64 running = 0;
    i64 s2h = 0;
    for (u64 low = 0; low <= z; low += SEGN) {
        const u64 high = std::min(low + SEGN, z + 1);
        const u32 nbytes = (u32)((high - low + 29) / 30);  // [low, high) に掛かるバイト
        const u32 nw = (nbytes + 7) / 8;
        {
            u32 r = (u32)((low / 30) % 1001);
            u32 k = 0;
            while (k < nbytes) {
                u32 len = std::min<u32>(1001 - r, nbytes - k);
                std::memcpy(seg + k, pat.data() + r, len);
                k += len;
                r = 0;
            }
            for (u32 j = nbytes; j < 8 * (SWORDS + 1); ++j) seg[j] = 0;
            // high 以上の数のビットを落とす
            const u64 last = high - 1 - low;  // 区間の中で最後の数の位置
            seg64[last / 240] &= MASK[last % 240];
        }
        // 17 から p_c までは数えずに消し、残りの個数をまとめて数える
        for (u64 b = 7; b <= c; ++b) {
            u64 dummy = 0;
            cross_off<false>(seg, nbytes, sp[b], dummy);
        }
        u64 cnt = 0;
        for (u32 k = 0; k < nw; ++k) cnt += (u64)std::popcount(seg64[k]);

        // 葉の t は zh 以下なので、zh より先の区間は数えずに消すだけ
        const bool hard = low <= zh;
        for (u64 b = c + 1; b <= bmax; ++b) {
            if (!hard) {
                u64 dummy = 0;
                cross_off<false>(seg, nbytes, sp[b], dummy);
                continue;
            }
            const u64 Nb = N[b];
            // この区間に入る葉は t < high、つまり m > Nb / high のもの。
            // 素数の m は添字 (pj, pi_cur]、合成数の m は添字 (cj, ci_cur] (lpf > p だけ)。
            const u64 mb = Nb / high;
            const u32 pi_hi = pi_cur[b];
            const u32 pj = std::max<u32>(pi_lo[b], std::min<u32>(pi_hi, (u32)npr_upto(mb)));
            const int64_t ci_hi = ci_cur[b];
            const int64_t cj = std::max<int64_t>(
                ci_lo[b] - 1, std::min<int64_t>(ci_hi, (int64_t)(std::upper_bound(comp_m.begin(), comp_m.end(), (u32)std::min<u64>(mb, w)) - comp_m.begin()) - 1));
            if (pj < pi_hi || cj < ci_hi) {
                // 最も大きい t の語まで、語ごとの累積を作る
                u64 tmax = 0;
                if (pj < pi_hi) tmax = mulhi(Nb, pmagic[pj + 1]);
                if (cj < ci_hi) tmax = std::max(tmax, mulhi(Nb, cmagic[cj + 1]));
                const u32 wl = (u32)((tmax - low) / 240);
                u64 run = 0;
                for (u32 w = 0; w <= wl; ++w) {
                    pre[w] = (u32)run;
                    run += (u64)std::popcount(seg64[w]);
                }
                i64 sum = 0;
                for (u32 i = pi_hi; i > pj; --i) {
                    const u64 u = mulhi(Nb, pmagic[i]) - low;
                    const u32 w = (u32)(u / 240);
                    sum += (i64)pre[w] + std::popcount(seg64[w] & MASK[u - 240 * (u64)w]);
                }
                s2h += sum + (i64)(pi_hi - pj) * phi[b];
                pi_cur[b] = pj;
                const u32 p = primes[b];
                for (int64_t i = ci_hi; i > cj; --i) {
                    if (comp_lpf[i] <= p) continue;
                    const u64 u = mulhi(Nb, cmagic[i]) - low;
                    const u32 w = (u32)(u / 240);
                    const i64 ph = phi[b] + (i64)pre[w] + std::popcount(seg64[w] & MASK[u - 240 * (u64)w]);
                    s2h += comp_mu[i] > 0 ? -ph : ph;
                }
                ci_cur[b] = cj;
            }
            phi[b] += (i64)cnt;
            // p_b の倍数を消しながら、消えた数を cnt から引く
            cross_off<true>(seg, nbytes, sp[b], cnt);
        }
        for (u64 b = bmax + 1; b <= nsz; ++b) {
            u64 dummy = 0;
            cross_off<false>(seg, nbytes, sp[b], dummy);
        }
        if (low == 0) {
            seg[0] &= (u8)~1u;  // 1 は素数でない
            for (u64 b = 4; b <= std::max<u64>(bmax, c); ++b) {  // 7 以上で消した素数を戻す
                const u64 p = primes[b];
                seg[p / 30] |= (u8)(1u << BI[p % 30]);
            }
        }
        const u64 w0 = low / 240;
        for (u32 k = 0; k < nw; ++k) {
            pt.t[w0 + k].bits = seg64[k];
            pt.t[w0 + k].cnt = running;
            running += (u64)std::popcount(seg64[k]);
        }
    }
    DR_MARK(T2);

    // easy leaves と trivial leaves
    i64 s2e = 0;
    for (u64 b = c + 1; b < a; ++b) {
        const u64 p = primes[b];
        const u64 Nb = x / p;
        const u64 p3 = Nb / (p * p);
        // 合成数の m ∈ (max(w/p, x/p^3), w] (p^2 < m ≤ w なので p < √w のときだけ)
        if (p * p < w) {
            const u64 loc = std::min(w, std::max(w / p, p3));
            size_t i = std::upper_bound(comp_m.begin(), comp_m.end(), (u32)loc) - comp_m.begin();
            for (; i < comp_m.size(); ++i) {
                if (comp_lpf[i] <= p) continue;
                const u64 t = mulhi(Nb, cmagic[i]);
                const i64 ph = t < p ? 1 : (i64)pt(t) - (i64)b + 2;
                s2e += comp_mu[i] > 0 ? -ph : ph;
            }
        }
        // 素数の q ∈ (lo, y]: q ≤ he なら easy、q > he なら trivial
        const u64 lo = std::max({w / p, p, p3});
        if (lo >= y) continue;
        const u64 he = std::min(y, Nb / p);
        if (he < y) s2e += (i64)(a - pt(std::max(lo, he)));
        if (lo >= he) continue;
        const u64 s = isqrt(Nb);
        const u64 ilo = pt(lo), ihe = pt(he);
        // f(i) = π(Nb / q_i) の (l, r] の和
        auto acc = [&](u64 l, u64 r) -> i64 {
            i64 v = 0;
            for (u64 i = l + 1; i <= r; ++i) v += (i64)pt(mulhi(Nb, pmagic[i]));
            return v;
        };
        // sparse: q ∈ (lo, min(he, s)] は添字 (ilo, imid]
        const u64 imid = std::min(ihe, pt(std::max(lo, std::min(he, s))));
        i64 sum = (i64)(imid - ilo) * (2 - (i64)b);
        // clustered: q ∈ (al, he]、al = max(lo, s) を反転して r ∈ (Nb / he, Nb / al] の和にする。
        // r の添字 (rlo, rhi] のうち sparse の範囲に入る部分は、同じ f を 2 回足す。
        const u64 al = std::max(lo, s);
        u64 rlo = imid, rhi = imid;
        if (al < he) {
            const u64 pal = pt(al);
            const u64 nb = Nb / he, na = Nb / al;
            sum += (i64)(ihe * pt(nb)) - (i64)(pal * pt(na)) + (i64)(ihe - pal) * (2 - (i64)b);
            rlo = pt(nb);
            rhi = pt(na);
        }
        const u64 o1 = std::clamp(rlo, ilo, imid), o2 = std::clamp(rhi, o1, imid);
        sum += acc(ilo, o1) + 2 * acc(o1, o2) + acc(o2, imid);
        if (rlo < ilo) sum += acc(rlo, std::min(rhi, ilo));
        if (rhi > imid) sum += acc(std::max(rlo, imid), rhi);
        s2e += sum;
    }
    DR_MARK(T3);

    // ordinary leaves: n = 1、p_c < q ≤ y の素数、合成数
    Phi8 phic;
    i64 s1 = (i64)phic(x);
    for (u64 i = c + 1; i <= a; ++i) s1 -= (i64)phic(fdiv(x, primes[i]));
    for (size_t i = 0; i < comp_m.size(); ++i) {
        const i64 ph = (i64)phic(fdiv(x, comp_m[i]));
        s1 += comp_mu[i] > 0 ? ph : -ph;
    }
    DR_MARK(T4);

    // P2: y < p ≤ √x の素数を π の表のビットから列挙する
    i64 p2 = 0;
    {
        u64 b = a;
        const u64 wlo = (y + 1) / 240, whi = sx / 240;
        for (u64 w = wlo; w <= whi; ++w) {
            u64 bits = pt.t[w].bits;
            while (bits) {
                const u32 k = (u32)std::countr_zero(bits);
                bits &= bits - 1;
                const u64 p = 240 * w + 30 * (k >> 3) + WR[k & 7];
                if (p <= y) continue;
                if (p > sx) break;
                ++b;
                p2 += (i64)pt(fdiv(x, p)) - (i64)b + 1;
            }
        }
    }
    DR_MARK(T5);
#ifdef DR_PROF
    fprintf(stderr, "y=%llu z=%llu a=%llu bmax=%llu | small %.3f pass %.3f easy %.3f s1 %.3f p2 %.3f\n",
            (unsigned long long)y, (unsigned long long)z, (unsigned long long)a, (unsigned long long)bmax,
            DR_MS(T0, T1), DR_MS(T1, T2), DR_MS(T2, T3), DR_MS(T3, T4), DR_MS(T4, T5));
#endif
    return (u64)(s1 + s2h + s2e + (i64)a - 1 - p2);
}

}  // namespace dr3
