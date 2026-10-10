#pragma once
// Deléglise–Rivat の素数の個数 π(x) (7 回目)。_dr6.hpp の割り算を magic のまま (dr6_mg と同じ)、π の表を奇数だけにし、
// easy leaves の表引きを AVX2 で 4 つずつ回す。商 floor(N / q) は 1 / q を 1 ulp 上げた double との掛け算で
// 4 つまとめて求め (N < 2^50 で正確)、表は 1 回の gather で引く。π(t) は表の値 e の上位 32 bit と、
// e を 63 - ((t - 1) / 2 mod 32) だけ左にずらした値の popcount (pshufb の 4 bit の表と psadbw) の和に 1 を足したもの。
// x86 では immintrin.h、ほかでは SIMDe で組む。
//
// 式と葉の分け方は _dr5.hpp と同じ。y = α x^{1/3}、z = x / y、a = π(y)、c = 8 (p_c = 19) として
//   π(x) = S1 + S2 + a - 1 - P2
//   S1 = Σ_{m ≤ y, 平方因子なし, lpf(m) > p_c} μ(m) φ(x / m, c)                          (ordinary leaves)
//   S2 = Σ_{c < b < a} Σ_{y / p_b < m ≤ y, 平方因子なし, lpf(m) > p_b} -μ(m) φ(x / (p_b m), b - 1)  (special leaves)
//   P2 = Σ_{y < p_b ≤ √x} (π(x / p_b) - b + 1)
// special leaves は t = x / (p_b m) で分け、t ≥ p_b^2 (hard) は [1, z] を区間ごとに篩って数え、p_b ≤ t < p_b^2 (easy)
// は π の表を引き、t < p_b (trivial) は個数だけ足す。
#include "../common.hpp"
#ifdef __x86_64__
#include <immintrin.h>
#else
#include <simde/x86/avx2.h>
#endif

namespace dr7 {

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
// n < 2^53 なら double で割る。正しく丸めた商は floor(n / d) か 1 大きいだけなので、1 回直せばよい。
inline u64 fdiv(u64 n, u64 d) {
    if (n < (u64(1) << 53)) {
        const u64 q = (u64)(i64)((double)n / (double)d);
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
    for (u32 j = 0; j < 240; ++j)
        for (u32 k = 0; k < 8; ++k)
            for (u32 i = 0; i < 8; ++i)
                if (30 * k + WR[i] <= j) t[j] |= u64(1) << (8 * k + i);
    return t;
}();
// p ≡ WR[c] (mod 30) の倍数 p m (m ≡ WR[i]) のビットの位置と、m が 30 進む 1 周の先頭からのバイトの差の端数
inline constexpr std::array<std::array<u8, 8>, 8> WBIT = [] {
    std::array<std::array<u8, 8>, 8> t{};
    for (u32 c = 0; c < 8; ++c)
        for (u32 i = 0; i < 8; ++i) t[c][i] = BI[WR[c] * WR[i] % 30];
    return t;
}();
inline constexpr std::array<std::array<u8, 8>, 8> WOFF = [] {
    std::array<std::array<u8, 8>, 8> t{};
    for (u32 c = 0; c < 8; ++c)
        for (u32 i = 0; i < 8; ++i) t[c][i] = (u8)(WR[c] * WR[i] / 30);
    return t;
}();

// 奇数だけの π の表。t[k] の下位 32 bit の bit j は 64 k + 2 j + 1 が素数か、上位 32 bit は 64 k 未満の奇素数の個数。
// 引くときに 240 で割らずに済み、位置は shift だけで決まる。
struct PiOdd {
    std::vector<u64> t;
    // 2 ≤ n < 2^32
    u64 operator()(u64 n) const {
        const u32 m = (u32)n - 1;
        const u64 e = t[m >> 6];
        const u32 pc = (u32)std::popcount((u32)e << (~m >> 1 & 31)) + 1;  // u32 で足すと符号の拡張が入らない
        return (e >> 32) + pc;
    }
};
// 30 の車輪の 1 バイトを、奇数の位置 (30 k + r の r = 1, 7, ..., 29 は (r - 1) / 2 番目) の 15 bit に広げる
inline constexpr std::array<uint16_t, 256> EXP15 = [] {
    std::array<uint16_t, 256> t{};
    for (u32 v = 0; v < 256; ++v)
        for (u32 i = 0; i < 8; ++i)
            if (v >> i & 1) t[v] |= (uint16_t)(1u << (WR[i] - 1) / 2);
    return t;
}();

// φ(t, 6) を 30030 周期の表で引き、φ(t, 8) = φ(t, 6) - φ(t / 17, 6) - φ(t / 19, 6) + φ(t / 323, 6)
struct Phi8 {
    std::vector<uint16_t> tab;
    Phi8() : tab(30030) {
        std::vector<u8> co(30030, 1);
        co[0] = 0;
        for (u32 p : {2u, 3u, 5u, 7u, 11u, 13u})
            for (u32 j = p; j < 30030; j += p) co[j] = 0;
        u32 c = 0;
        for (u32 i = 0; i < 30030; ++i) tab[i] = (uint16_t)(c += co[i]);
    }
    u64 phi6(u64 t) const { return t / 30030 * 5760 + tab[t % 30030]; }
    u64 operator()(u64 t) const { return phi6(t) - phi6(t / 17) - phi6(t / 19) + phi6(t / 323); }
};

// 篩う素数の状態。pos は今の区間の先頭からのバイトの位置、i は倍数 p m の m の車輪の位置。
struct SievingPrime {
    u32 pos, a;
    u8 cls, i;
};

// 区間の [0, nbytes) バイトにある p の倍数を消し、COUNT なら消えた数を返す。消えた数は局所変数に数える。
template <u32 C, bool COUNT>
inline u32 cross_cls(u8* seg, u32 nbytes, SievingPrime& s) {
    const u32 a = s.a, p = 30 * a + WR[C];
    const u32 off[8] = {0, a * 6 + WOFF[C][1], a * 10 + WOFF[C][2], a * 12 + WOFF[C][3],
                        a * 16 + WOFF[C][4], a * 18 + WOFF[C][5], a * 22 + WOFF[C][6], a * 28 + WOFF[C][7]};
    u32 gone = 0;
    auto clr = [&](u32 q, u32 bit) {
        if constexpr (COUNT) {
            const u8 v = seg[q];
            gone += (v >> bit) & 1;
            seg[q] = (u8)(v & ~(1u << bit));
        } else {
            seg[q] &= (u8)~(1u << bit);
        }
    };
    u32 i = s.i, base = s.pos - off[i];  // 周の先頭 (区間より前なら 2^32 で回る)
    for (; i < 8; ++i) {
        const u32 q = base + off[i];
        if (q >= nbytes) return s.pos = q - nbytes, s.i = (u8)i, gone;
        clr(q, WBIT[C][i]);
    }
    for (base += p; base + off[7] < nbytes; base += p) {
        clr(base, WBIT[C][0]), clr(base + off[1], WBIT[C][1]), clr(base + off[2], WBIT[C][2]), clr(base + off[3], WBIT[C][3]);
        clr(base + off[4], WBIT[C][4]), clr(base + off[5], WBIT[C][5]), clr(base + off[6], WBIT[C][6]), clr(base + off[7], WBIT[C][7]);
    }
    for (i = 0;; ++i) {
        const u32 q = base + off[i];
        if (q >= nbytes) return s.pos = q - nbytes, s.i = (u8)i, gone;
        clr(q, WBIT[C][i]);
    }
}
template <bool COUNT>
inline u32 cross_off(u8* seg, u32 nbytes, SievingPrime& s) {
    switch (s.cls) {
        case 0: return cross_cls<0, COUNT>(seg, nbytes, s);
        case 1: return cross_cls<1, COUNT>(seg, nbytes, s);
        case 2: return cross_cls<2, COUNT>(seg, nbytes, s);
        case 3: return cross_cls<3, COUNT>(seg, nbytes, s);
        case 4: return cross_cls<4, COUNT>(seg, nbytes, s);
        case 5: return cross_cls<5, COUNT>(seg, nbytes, s);
        case 6: return cross_cls<6, COUNT>(seg, nbytes, s);
        default: return cross_cls<7, COUNT>(seg, nbytes, s);
    }
}

// Σ_{l < i ≤ r} π(floor(n inv[i]))。4 つずつ AVX2 で回し、端は 1 つずつ。
inline i64 sum_pi4(const PiOdd& pt, const double* inv, u64 l, u64 r, double n) {
    u64 i = l + 1;
    i64 s = 0;
    if (i + 3 <= r) {
        const __m256d vn = _mm256_set1_pd(n);
        const __m256i one = _mm256_set1_epi64x(1), c31 = _mm256_set1_epi64x(31), c63 = _mm256_set1_epi64x(63), m4 = _mm256_set1_epi8(0x0f);
        const __m256i lut = _mm256_setr_epi8(0, 1, 1, 2, 1, 2, 2, 3, 1, 2, 2, 3, 2, 3, 3, 4, 0, 1, 1, 2, 1, 2, 2, 3, 1, 2, 2, 3, 2, 3, 3, 4);
        const long long* tab = (const long long*)pt.t.data();
        __m256i acc = _mm256_setzero_si256();
        for (; i + 3 <= r; i += 4) {
            const __m256i t = _mm256_cvtepu32_epi64(_mm256_cvttpd_epi32(_mm256_mul_pd(vn, _mm256_loadu_pd(inv + i))));
            const __m256i m = _mm256_sub_epi64(t, one);
            const __m256i e = _mm256_i64gather_epi64(tab, _mm256_srli_epi64(m, 6), 8);
            // 下位 32 bit の bit j (j ≤ (m / 2) mod 32) だけが残り、上位 32 bit の個数は押し出される
            const __m256i v = _mm256_sllv_epi64(e, _mm256_sub_epi64(c63, _mm256_and_si256(_mm256_srli_epi64(m, 1), c31)));
            const __m256i pc = _mm256_add_epi8(_mm256_shuffle_epi8(lut, _mm256_and_si256(v, m4)), _mm256_shuffle_epi8(lut, _mm256_and_si256(_mm256_srli_epi16(v, 4), m4)));
            acc = _mm256_add_epi64(acc, _mm256_add_epi64(_mm256_sad_epu8(pc, _mm256_setzero_si256()), _mm256_srli_epi64(e, 32)));
        }
        alignas(32) u64 a4[4];
        _mm256_store_si256((__m256i*)a4, acc);
        s = (i64)(a4[0] + a4[1] + a4[2] + a4[3] + (i - l - 1));  // 1 つにつき 2 の分の 1
    }
    for (; i <= r; ++i) s += (i64)pt((u64)(i64)(n * inv[i]));
    return s;
}

// sum_pi4 と同じ和。商だけ 4 つずつ AVX2 で求め、表は 1 つずつ引く (gather を使わない)。
inline i64 sum_pi4s(const PiOdd& pt, const double* inv, u64 l, u64 r, double n) {
    u64 i = l + 1;
    i64 s = 0;
    const __m256d vn = _mm256_set1_pd(n);
    alignas(16) u32 tb[4];
    for (; i + 3 <= r; i += 4) {
        _mm_store_si128((__m128i*)tb, _mm256_cvttpd_epi32(_mm256_mul_pd(vn, _mm256_loadu_pd(inv + i))));
        s += (i64)(pt(tb[0]) + pt(tb[1]) + pt(tb[2]) + pt(tb[3]));
    }
    for (; i <= r; ++i) s += (i64)pt((u64)(i64)(n * inv[i]));
    return s;
}

// 奇数だけの篩で数える。小さい x 用。
inline u64 pi_small(u64 n) {
    if (n < 2) return 0;
    u64 h = (n - 1) / 2, cnt = 1;
    std::vector<u8> comp(h + 1, 0);
    for (u64 i = 1; i <= h; ++i)
        if (!comp[i]) {
            ++cnt;
            for (u64 p = 2 * i + 1, j = (p * p) / 2; j <= h; j += p) comp[j] = 1;
        }
    return cnt;
}

// am は primecount の α の当てはめに掛ける倍率。easy leaves の表引きは、MODE が 0 なら 1 つずつ、1 なら AVX2 で
// 4 つずつ (gather)、2 なら商だけ AVX2 で 4 つずつ求めて表は 1 つずつ引く。
template <int MODE = 1>
inline u64 prime_pi(u64 x, double am = 1.5) {
    if (x < 100000) return pi_small(x);
    // n / d は floor(2^64 / d) + 1 との掛け算の上位 (n d < 2^64 で正確)。AVX2 の表引きだけ double で割る。
    using D = u64;
    auto mkd = [](u64 d) -> D { return magic(d); };
    auto num = [](u64 n) -> D { return n; };
    auto qt = [](D n, D dv) -> u64 { return mulhi(n, dv); };
    constexpr u32 c = 8;
    const double L = std::log((double)x);
    const double alpha = std::max(1.0, am * (((0.00148918 * L - 0.0691909) * L + 1.00165) * L + 0.372253));
    const u64 x13 = icbrt(x), sx = isqrt(x);
    const u64 y = std::min(sx, std::max(x13 + 1, (u64)(alpha * (double)x13))), z = x / y, sz = isqrt(z);
    // y + 2 までの素数 (1 始まり)。反転で引く r は √(x / p) + 2 以下で、反転があるのは √(x / p) < y のときだけ。
    std::vector<u32> primes;
    {
        const u64 L2 = y + 2, h = (L2 - 1) / 2;
        std::vector<u8> comp(h + 1, 0);
        for (u64 i = 1; (2 * i + 1) * (2 * i + 1) <= L2; ++i)
            if (!comp[i])
                for (u64 p = 2 * i + 1, j = (p * p) / 2; j <= h; j += p) comp[j] = 1;
        primes.resize(h + 2), primes[0] = 0, primes[1] = 2;
        size_t k = 2;
        for (u64 i = 1; i <= h; ++i) primes[k] = (u32)(2 * i + 1), k += !comp[i];
        primes.resize(k);
    }
    auto npr = [&](u64 n) -> u64 { return (u64)(std::upper_bound(primes.begin() + 1, primes.end(), (u32)n) - primes.begin()) - 1; };
    const u64 a = npr(y), nsz = npr(sz);
    // y 以下の平方因子のない合成数で lpf > p_c のもの (昇順)。素数の積を深さ優先でたどって表に lpf と μ を書き、小さい順に詰める。
    std::vector<u32> cm, clp;
    std::vector<int8_t> cmu;
    {
        std::vector<uint16_t> tag(y + 1, 0);  // lpf | (μ > 0 ? 0x8000 : 0)。合成数の lpf は √y 以下
        size_t cnt = 0;
        auto dfs = [&](auto& self, u64 m, size_t i, u32 lp, int mu) -> void {
            for (; i <= a; ++i) {
                const u64 mm = m * primes[i];
                if (mm > y) break;
                if (lp) tag[mm] = (uint16_t)(lp | (mu > 0 ? 0x8000 : 0)), ++cnt;
                self(self, mm, i + 1, lp ? lp : primes[i], -mu);
            }
        };
        dfs(dfs, 1, c + 1, 0, 1);
        cm.resize(cnt + 1);
        size_t k = 0;
        for (u64 m = 1; m <= y; ++m) cm[k] = (u32)m, k += tag[m] != 0;
        cm.resize(cnt), clp.resize(cnt), cmu.resize(cnt);
        for (size_t i = 0; i < cnt; ++i) clp[i] = tag[cm[i]] & 0x7fff, cmu[i] = (tag[cm[i]] & 0x8000) ? -1 : 1;
    }
    std::vector<D> pd(primes.size()), cd(cm.size());
    for (size_t i = 1; i < primes.size(); ++i) pd[i] = mkd(primes[i]);
    for (size_t i = 0; i < cm.size(); ++i) cd[i] = mkd(cm[i]);
    std::vector<double> pinv;  // 1 / p を 1 ulp 上げた値。N < 2^50 なら floor(N pinv) = floor(N / p)
    if constexpr (MODE != 0) {
        pinv.resize(primes.size());
        for (size_t i = 1; i < primes.size(); ++i) pinv[i] = std::nextafter(1.0 / primes[i], 2.0);
    }
    const D xD = num(x);
    // hard leaves: b ごとに m ∈ (max(y/p, p), min(y, x/p^3)]。素数の m は primes の添字 (plo, pcur] で持ち、上から下る
    // (t が増える向き)。範囲が空でないなら p^2 ≤ z。
    u64 bmax = c;
    std::vector<u64> N(a + 1, 0);
    std::vector<u32> pcur(a + 1, 0), plo(a + 1, 0);
    for (u64 b = c + 1; b < a; ++b) {
        const u64 p = primes[b];
        if (p * p > z) break;
        const u64 lo = std::max(y / p, p), hi = std::min(y, x / (p * p * p));
        if (lo >= hi) continue;
        bmax = b, N[b] = x / p, plo[b] = (u32)npr(lo), pcur[b] = (u32)npr(hi);
    }
    // 合成数の m の hard leaves は、b の分を ht[hcb[b], hcb[b + 1]) に t の昇順で並べる。hneg は μ(m) > 0 (-μ φ を足す
    // ので引く) かどうか。lpf > p_b の合成数の列 alive から lpf = p_b のものを抜きながら b を進め、(lo, hi] を後ろから写す。
    std::vector<u32> hcb(bmax + 2, 0), ht;
    std::vector<u8> hneg;
    {
        std::vector<u32> alive(cm.size());
        for (size_t i = 0; i < cm.size(); ++i) alive[i] = (u32)i;
        for (u64 b = c + 1; b <= bmax; ++b) {
            hcb[b] = (u32)ht.size();
            const u64 p = primes[b];
            size_t k = 0;
            for (size_t j = 0; j < alive.size(); ++j) alive[k] = alive[j], k += clp[alive[j]] > p;
            alive.resize(k);
            if (alive.empty() || !N[b]) continue;
            const u64 lo = std::max(y / p, p), hi = std::min(y, x / (p * p * p));
            auto gt = [&](u32 v, u32 i) { return v < cm[i]; };
            const auto i1 = std::upper_bound(alive.begin(), alive.end(), (u32)lo, gt), i2 = std::upper_bound(i1, alive.end(), (u32)hi, gt);
            const D NbD = num(N[b]);
            for (auto it = i2; it != i1;) --it, ht.push_back((u32)qt(NbD, cd[*it])), hneg.push_back(cmu[*it] > 0);
        }
        hcb[bmax + 1] = (u32)ht.size();
    }
    std::vector<u32> hcur(hcb.begin(), hcb.end());
    // 区間の篩。区間の先頭 low は 240 の倍数で、バイト k は 30 (low / 30 + k) + WR[i]。
    constexpr u32 SW = 2048;
    constexpr u64 SEGN = 240 * (u64)SW;
    std::vector<u64> seg64(SW + 1, 0);
    u8* seg = reinterpret_cast<u8*>(seg64.data());
    std::vector<u8> pat(1001);  // 7, 11, 13 の倍数を除く型
    for (u32 k = 0; k < 1001; ++k)
        for (u32 i = 0; i < 8; ++i) {
            const u64 n = 30 * (u64)k + WR[i];
            if (n % 7 && n % 11 && n % 13) pat[k] |= (u8)(1u << i);
        }
    // 17 以上の素数で篩う。p_c までと hard leaves のある素数は φ のために p 自身から、残りは π の表のためだけなので p^2 から消す。
    const u64 nsp = std::max<u64>(nsz, c);
    std::vector<SievingPrime> sp(std::max(nsp, bmax) + 1);
    for (u64 b = 7; b <= nsp; ++b) {
        const u64 p = primes[b], m0 = b <= std::max<u64>(bmax, c) ? 1 : p;
        sp[b] = SievingPrime{(u32)(p * m0 / 30), (u32)(p / 30), BI[p % 30], BI[m0 % 30]};
    }
    std::vector<i64> phi(bmax + 1, 0);
    std::vector<u32> pre(SW + 1);
    PiOdd pt;
    pt.t.resize(z / 64 + 1);
    u64 running = 0;
    i64 s2h = 0;
    for (u64 low = 0; low <= z; low += SEGN) {
        const u64 high = std::min(low + SEGN, z + 1);
        const u32 nbytes = (u32)((high - low + 29) / 30), nw = (nbytes + 7) / 8;
        for (u32 k = 0, r = (u32)(low / 30 % 1001); k < nbytes; r = 0) {
            const u32 len = std::min<u32>(1001 - r, nbytes - k);
            std::memcpy(seg + k, pat.data() + r, len), k += len;
        }
        std::memset(seg + nbytes, 0, 8 * (SW + 1) - nbytes);
        seg64[(high - 1 - low) / 240] &= MASK[(high - 1 - low) % 240];  // high 以上の数のビットを落とす
        u64 cnt = 0;
        for (u64 b = 7; b <= c; ++b) cross_off<false>(seg, nbytes, sp[b]);  // 17 と 19 は数えずに消す
        for (u32 k = 0; k < nw; ++k) cnt += (u64)std::popcount(seg64[k]);
        for (u64 b = c + 1; b <= bmax; ++b) {
            // この区間に入る葉は t < high のもの。素数の m は添字 (pj, pih]、合成数は ht の [hc0, hk)。
            const u32 pih = pcur[b], hc0 = hcur[b], hc1 = hcb[b + 1];
            if (pih > plo[b] || hc0 < hc1) {
                const D NbD = num(N[b]);
                u32 pj = pih;
                if (pih > plo[b] && qt(NbD, pd[pih]) < high) {
                    const u32 mb = (u32)(N[b] / high);  // t < high ⇔ m > mb
                    pj = (u32)(std::upper_bound(primes.begin() + plo[b] + 1, primes.begin() + pih + 1, mb) - primes.begin()) - 1;
                }
                const u32 hk = (u32)(std::partition_point(ht.begin() + hc0, ht.begin() + hc1, [&](u32 t) { return t < high; }) - ht.begin());
                if (pj < pih || hc0 < hk) {
                    // 最も大きい t の語まで、語ごとの累積を作ってから葉を数える
                    u64 tmax = 0;
                    if (pj < pih) tmax = qt(NbD, pd[pj + 1]);
                    if (hc0 < hk) tmax = std::max<u64>(tmax, ht[hk - 1]);
                    const u32 wl = (u32)((tmax - low) / 240);
                    u32 run = 0;
                    for (u32 w = 0; w <= wl; ++w) pre[w] = run, run += (u32)std::popcount(seg64[w]);
                    i64 sum = 0;
                    for (u32 i = pih; i > pj; --i) {
                        const u32 u = (u32)(qt(NbD, pd[i]) - low), w = u / 240;
                        sum += (i64)pre[w] + std::popcount(seg64[w] & MASK[u - 240 * w]);
                    }
                    s2h += sum + (i64)(pih - pj) * phi[b], pcur[b] = pj;
                    for (u32 k = hc0; k < hk; ++k) {
                        const u32 u = ht[k] - (u32)low, w = u / 240;
                        const i64 ph = phi[b] + (i64)pre[w] + std::popcount(seg64[w] & MASK[u - 240 * w]);
                        s2h += hneg[k] ? -ph : ph;
                    }
                    hcur[b] = hk;
                }
            }
            phi[b] += (i64)cnt;
            cnt -= cross_off<true>(seg, nbytes, sp[b]);  // p_b の倍数を消し、消えた数を cnt から引く
        }
        for (u64 b = bmax + 1; b <= nsz; ++b) cross_off<false>(seg, nbytes, sp[b]);
        if (low == 0) {
            seg[0] &= (u8)~1u;                                                                                      // 1 は素数でない
            for (u64 b = 4; b <= std::max<u64>(bmax, c); ++b) seg[primes[b] / 30] |= (u8)(1u << BI[primes[b] % 30]);  // 消した素数を戻す
        }
        {
            // 奇数の位置の列は 1 バイトにつき 15 bit 進む。32 bit たまるごとに書く。
            const u32 ne = (u32)((high - low + 63) / 64);
            u64* out = pt.t.data() + low / 64;
            u64 acc = 0;
            u32 nacc = 0;
            if (low == 0) acc = 6;  // 3 と 5 は車輪の外なので足す (位置 1 と 2)
            for (u32 k = 0, o = 0; o < ne; ++k) {
                acc |= (u64)EXP15[seg[k]] << nacc, nacc += 15;
                if (nacc >= 32) {
                    const u32 v = (u32)acc;
                    out[o++] = running << 32 | v, running += (u64)std::popcount(v), acc >>= 32, nacc -= 32;
                }
            }
        }
    }
    // easy leaves と trivial leaves。p_b > max(x^{1/3}, √y) の b は葉が q ∈ (p_b, y] の trivial だけで、b の分は a - b。
    i64 s2e = 0;
    u64 b0 = c + 1;
    while (b0 < a && !(primes[b0] * primes[b0] * primes[b0] > x && y < primes[b0] * (primes[b0] + 1))) ++b0;
    if (b0 < a) s2e += (i64)((a - b0) * (a - b0 + 1) / 2);
    const D yD = num(y);
    for (u64 b = c + 1; b < b0; ++b) {
        const u64 p = primes[b];
        const D dv = pd[b];
        const u64 Nb = qt(xD, dv), Nbp = qt(num(Nb), dv), lo = std::max({qt(yD, dv), p, qt(num(Nbp), dv)});  // 素数の q も合成数の m も lo より大きい
        if (lo >= y) continue;
        const D NbD = num(Nb);
        // 合成数の m = q1 q2 (p < q1 < q2、lo < m ≤ y)。t ≥ p (p < √y ≤ √z) なので trivial はなく、μ(m) = 1 なので引く。
        for (u64 j = b + 1; primes[j] * primes[j + 1] <= y; ++j) {
            const u64 q1 = primes[j], k1 = npr(y / q1);
            const D Nq = num(qt(NbD, pd[j]));
            for (u64 k = std::max<u64>(j, npr(lo / q1)) + 1; k <= k1; ++k) s2e -= (i64)pt(qt(Nq, pd[k])) - (i64)b + 2;
        }
        // 素数の q ∈ (lo, y]: q ≤ he なら easy、q > he なら trivial
        const u64 he = std::min(y, Nbp);
        if (he < y) s2e += (i64)(a - pt(std::max(lo, he)));
        if (lo >= he) continue;
        const u64 s = isqrt(Nb), ilo = pt(lo), ihe = pt(he), imid = std::min(ihe, pt(std::max(lo, std::min(he, s))));
        auto acc = [&](u64 l, u64 r) -> i64 {  // π(Nb / q_i) の (l, r] の和
            if constexpr (MODE == 1) {
                return sum_pi4(pt, pinv.data(), l, r, (double)Nb);
            } else if constexpr (MODE == 2) {
                return sum_pi4s(pt, pinv.data(), l, r, (double)Nb);
            } else {
                i64 v = 0;
                for (u64 i = l + 1; i <= r; ++i) v += (i64)pt(qt(NbD, pd[i]));
                return v;
            }
        };
        // sparse: q ∈ (lo, min(he, s)]。clustered: q ∈ (al, he] を反転して r ∈ (Nb / he, Nb / al] の和にし、
        // r の添字 (rlo, rhi] のうち sparse の範囲に入る部分は、同じ表引きを 2 回足す。
        i64 sum = (i64)(imid - ilo) * (2 - (i64)b);
        const u64 al = std::max(lo, s);
        u64 rlo = imid, rhi = imid;
        if (al < he) {
            const u64 pal = pt(al), nb = Nb / he, na = Nb / al;
            sum += (i64)(ihe * pt(nb)) - (i64)(pal * pt(na)) + (i64)(ihe - pal) * (2 - (i64)b), rlo = pt(nb), rhi = pt(na);
        }
        const u64 o1 = std::clamp(rlo, ilo, imid), o2 = std::clamp(rhi, o1, imid);
        sum += acc(ilo, o1) + 2 * acc(o1, o2) + acc(o2, imid);
        if (rlo < ilo) sum += acc(rlo, std::min(rhi, ilo));
        if (rhi > imid) sum += acc(std::max(rlo, imid), rhi);
        s2e += sum;
    }
    // ordinary leaves: m = 1、p_c < q ≤ y の素数、合成数
    Phi8 phic;
    i64 s1 = (i64)phic(x);
    for (u64 i = c + 1; i <= a; ++i) s1 -= (i64)phic(fdiv(x, primes[i]));
    for (size_t i = 0; i < cm.size(); ++i) {
        const i64 ph = (i64)phic(fdiv(x, cm[i]));
        s1 += cmu[i] > 0 ? ph : -ph;
    }
    // P2: y < p ≤ √x の素数を π の表のビットから列挙する
    i64 p2 = 0;
    for (u64 w = (y + 1) / 64, b = a; w <= sx / 64; ++w)
        for (u32 bits = (u32)pt.t[w]; bits; bits &= bits - 1) {
            const u64 p = 64 * w + 2 * (u32)std::countr_zero(bits) + 1;
            if (p <= y) continue;
            if (p > sx) break;
            p2 += (i64)pt(fdiv(x, p)) - (i64)(++b) + 1;
        }
    return (u64)(s1 + s2h + s2e + (i64)a - 1 - p2);
}

}  // namespace dr7
