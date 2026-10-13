#pragma once
// 離散対数の 5 回目。4 回目の機械語を直す。gcc は rho 法の歩みの指数の足し算 (a >= r ? r : 0) を分岐に戻していて、1 歩
// ごとに 5 割の確率で外れていた。5 回目は指数を歩むあいだ法 r で割らずに足し続け (3 万歩でも 2^45 未満)、区別点に着いた
// ときだけ割る。Barrett の掛け算からは m = 1 の確かめを外し (m = 1 は先に扱う)、累乗の掛ける値はマスクで選ぶ。
// 以下は 4 回目の説明。離散対数の 4 回目。3 回目の形で、位数 r が RHO_MIN より大きく、(Z/m)^* が巡回群 (m が素数冪かその 2 倍) のときは、
// 桁の離散対数を Pollard の rho 法で引く。r-adding walk (32 個の乗数 γ^{α_k} c^{β_k}、歩みの値のハッシュで選ぶ) を
// 16 本並べて進め、値が区別点 (ハッシュの下位の bit が 0) になったら表に (x, a, b) を入れる。別の (a', b') が同じ x に
// 着けば γ^{a} c^{b} = γ^{a'} c^{b'} から l を解く。掛け算の回数の期待値は √(π r / 2) ほどで baby-step giant-step と
// 変わらないが、大きい表を読み書きしない (区別点の表は数百個)。区別点に着かないまま長く歩んだ歩みは作り直す。
// 巡回群でない m では c が γ の冪とは限らず、rho 法が終わらないので baby-step giant-step を使う。
// 以下は 3 回目の説明。離散対数の 3 回目。2 回目の表の大きさを振る。B を √r の 2^{-BSH} 倍にすると、表は 4^{BSH} 分の 1 になり、引く回数の
// 期待値は 2^{BSH} 倍になる (掛け算の回数の合計 B + r / (2B) は BSH = 0 と 1 で同じ)。EPYC 7763 (L2 が 512 KB) では、
// 2 回目の B の 16 倍の表 (r が 5 億ほどで 2 MB) が 8 倍の表より遅かった。
// 以下は 2 回目の説明。離散対数の 2 回目。1 回目の baby-step giant-step の表を作り直す。項を 4 バイト (鍵のハッシュの 16 bit の指紋と、添字
// j + 1 の 16 bit) にし、表の大きさを B の HK 倍以上にとって、ほとんど空きにする。giant-step で引く鍵はほとんど表に無く、
// 線形探索で空きに当たるまでの回数が 1 回で済めば、引くたびの分岐が外れない (手元の M2 では、詰まり具合 1/2 の表より
// 16 倍の表のほうが 2 倍速かった)。指紋が合ったら ζ^j を累乗で作って確かめる。表の領域は問ごとに確保せず使い回す
// (B < 2^16)。B は √r、鎖は 8 本。
// 以下は 1 回目の説明。離散対数の 1 回目。X^K ≡ Y (mod M) となる最小の K を Pohlig-Hellman で求める。
//
// X と M が互いに素でない部分: K ≤ 32 は直に調べる。そこで見つからなければ、g = gcd(X, M) > 1 の間、Y が g で割り切れる
// ことを確かめて M、Y を g で割り、d に X / g を掛けて、段の数 c を数える (c ≤ 30)。残りは d X^{K'} ≡ Y (mod m)、
// gcd(X, m) = 1 で、答えは K = c + K' (K' は最小の解)。
//
// 互いに素な部分: m を factorize_pairs で分解して φ(m) の素因数分解を作り、X の位数 n を φ(m) から素因数を外していって
// 求める。T = Y d^{-1} について T^n ≠ 1 なら解は無い。n の素因数 r ごとに、γ = X^{n / r^e} (位数 r^e)、τ = T^{n / r^e}
// として、γ^x = τ の x を r 進の桁ごとに求める (桁は位数 r の ζ = γ^{r^{e-1}} の離散対数)。桁の離散対数は、r ≤ 64 なら
// 表を順に見て、それより大きければ baby-step giant-step で引く (表 ζ^j は r ごとに 1 回作り、掛け算の鎖は 4 本)。合成数の
// m では群が巡回群とは限らないので、桁が見つからなければ解は無い。x を CRT でまとめると [0, n) の最小の解になる。
//
// 剰余の掛け算は Barrett (m は偶数でもよい、m ≤ 10^9 なので積は 2^60 未満)。
#include "../common.hpp"
#include "neo/number_theory/factorize.hpp"
#include "neo/number_theory/inv_gcd.hpp"

namespace dl5 {

struct Barrett {
    u64 m, im;  // im = floor(2^64 / m) (m ≥ 2)
    explicit Barrett(u64 m_) : m(m_), im(m_ >= 2 ? (u64)(((u128)1 << 64) / m_) : 0) {}
    u64 mul(u64 a, u64 b) const {  // m ≥ 2
        const u64 t = a * b, q = (u64)((u128)t * im >> 64);
        u64 r = t - q * m;
        return r >= m ? r - m : r;
    }
    u64 pow(u64 b, u64 e) const {  // 掛ける値はマスクで選ぶ (gcc は三項演算子を分岐にする)
        u64 r = 1;
        for (; e; e >>= 1, b = mul(b, b)) r = mul(r, 1 ^ ((1 ^ b) & (0 - (e & 1))));
        return r;
    }
};

// 位数 r の ζ の冪 ζ^j (j < B) の表。c = ζ^l の l を引く (無ければ -1)。項は (指紋 16 bit << 16) | (j + 1)、0 は空き。
inline std::vector<u32>& table_buf() {
    static std::vector<u32> buf;
    return buf;
}
template <int HK, int BSH>
struct OrderRLog {
    const Barrett& M;
    u64 zeta, r, B, H, step;  // step = ζ^{-B}
    int lg;
    u32* tab = nullptr;
    OrderRLog(const Barrett& M_, u64 zeta_, u64 r_) : M(M_), zeta(zeta_), r(r_) {
        if (r <= 64) return;
        B = (((u64)std::sqrt((double)r) >> BSH) + 8) & ~7ull;
        lg = 4;
        while ((1ull << lg) < HK * B) ++lg;
        H = 1ull << lg;
        auto& buf = table_buf();
        if (buf.size() < H) buf.resize(H);
        tab = buf.data();
        std::memset(tab, 0, H * sizeof(u32));
        u64 zp[8];
        zp[0] = 1 % M.m;
        for (int d = 1; d < 8; ++d) zp[d] = M.mul(zp[d - 1], zeta);
        const u64 z8 = M.mul(zp[7], zeta);
        for (u64 j = 0; j < B; j += 8)
            for (int d = 0; d < 8; ++d) ins(zp[d], j + d), zp[d] = M.mul(zp[d], z8);
        step = M.pow(zp[0], r - 1);  // zp[0] = ζ^B
    }
    u64 hash(u64 k) const { return k * 0x9e3779b97f4a7c15ull; }
    void ins(u64 k, u64 j) {
        const u64 h = hash(k);
        u64 i = h >> (64 - lg);
        const u32 e = (u32)(h >> 16 & 0xffff0000u) | (u32)(j + 1);
        while (tab[i]) i = (i + 1) & (H - 1);
        tab[i] = e;
    }
    // 鍵 k の候補の j を順に試す
    long long look(u64 k) const {
        const u64 h = hash(k);
        const u32 fp = (u32)(h >> 16 & 0xffff0000u);
        for (u64 i = h >> (64 - lg); tab[i]; i = (i + 1) & (H - 1))
            if ((tab[i] & 0xffff0000u) == fp) {
                const u64 j = (tab[i] & 0xffffu) - 1;
                if (M.pow(zeta, j) == k) return (long long)j;
            }
        return -1;
    }
    long long operator()(u64 c) const {
        if (r <= 64) {
            u64 v = 1 % M.m;
            for (u64 l = 0; l < r; ++l, v = M.mul(v, zeta))
                if (v == c) return (long long)l;
            return -1;
        }
        u64 sp[8], u[8];
        sp[0] = 1 % M.m;
        for (int d = 1; d < 8; ++d) sp[d] = M.mul(sp[d - 1], step);
        const u64 s8 = M.mul(sp[7], step);
        for (int d = 0; d < 8; ++d) u[d] = M.mul(c, sp[d]);
        for (u64 i = 0; i <= r / B + 8; i += 8) {
            for (int d = 0; d < 8; ++d)
                if (const long long j = look(u[d]); j >= 0) return (long long)(((i + d) * B + j) % r);
            for (int d = 0; d < 8; ++d) u[d] = M.mul(u[d], s8);
        }
        return -1;
    }
};

// 素数 r の位数の zeta について、c = zeta^l の l を Pollard の rho 法 (区別点、16 本の歩み) で求める。c は zeta の冪とする。
inline u64 rho_log(const Barrett& M, u64 zeta, u64 c, u64 r) {
    if (c == 1) return 0;
    constexpr int K = 32, W = 16;
    u64 seed = r * 0x9e3779b97f4a7c15ull ^ c ^ (zeta << 7);
    auto rnd = [&] {
        u64 z = (seed += 0x9e3779b97f4a7c15ull);
        z = (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9ull;
        z = (z ^ (z >> 27)) * 0x94d049bb133111ebull;
        return z ^ (z >> 31);
    };
    u64 mx[K], ma[K], mb[K];
    for (int k = 0; k < K; ++k) ma[k] = rnd() % r, mb[k] = rnd() % r, mx[k] = M.mul(M.pow(zeta, ma[k]), M.pow(c, mb[k]));
    // 区別点の割合 2^{-t}: 1 本の歩みが区別点に着くまで √r / (8 W) 歩ほど
    int t = 4;
    while ((1ull << (2 * t + 2)) * 64 * W * W <= r) ++t;
    const u64 dmask = (1ull << t) - 1, maxlen = 20ull << t;
    // 区別点の表 (x → a, b)
    const int lg = 12;
    constexpr u32 EMPTY = ~0u;
    std::vector<u32> key(1u << lg, EMPTY), va(1u << lg), vb(1u << lg);
    int stored = 0;
    u64 x[W], a[W], b[W], len[W];
    auto start = [&](int w) {
        a[w] = rnd() % r, b[w] = rnd() % r, len[w] = 0;
        x[w] = M.mul(M.pow(zeta, a[w]), M.pow(c, b[w]));
    };
    for (int w = 0; w < W; ++w) start(w);
    for (;;) {
        for (int w = 0; w < W; ++w) {
            const int k = (int)((x[w] * 0xc2b2ae3d27d4eb4full) >> 59);
            x[w] = M.mul(x[w], mx[k]);
            a[w] += ma[k], b[w] += mb[k];  // 法 r では割らない (区別点で割る)
            ++len[w];
            if (((x[w] * 0x9e3779b97f4a7c15ull) >> 40 & dmask) == 0) [[unlikely]] {
                a[w] %= r, b[w] %= r;
                u32 h = (u32)((x[w] * 0xd6e8feb86659fd93ull) >> (64 - lg));
                while (key[h] != EMPTY && key[h] != x[w]) h = (h + 1) & ((1u << lg) - 1);
                if (key[h] == EMPTY) {
                    if (stored * 2 < (1 << lg)) key[h] = (u32)x[w], va[h] = (u32)a[w], vb[h] = (u32)b[w], ++stored;
                    len[w] = 0;
                } else if (vb[h] != b[w]) {
                    // γ^{a'} c^{b'} = γ^{a} c^{b} → c^{b - b'} = γ^{a' - a}
                    const u64 l = (u64)((u128)((va[h] + r - a[w]) % r) * ::inv_mod<u64>((b[w] + r - vb[h]) % r, r) % r);
                    if (M.pow(zeta, l) == c) return l;
                    start(w);
                } else
                    start(w);
            } else if (len[w] > maxlen) [[unlikely]]
                start(w);
        }
    }
}

// gcd(x, m) = 1 で x^k ≡ t (mod m) となる最小の k (無ければ -1)
template <int HK, int BSH, u64 RHO_MIN>
inline i64 coprime_log(u64 x, u64 t, u64 m) {
    if (m == 1) return 0;
    const Barrett M(m);
    bool cyclic;  // (Z/m)^* が巡回群か
    // φ(m) の素因数分解
    std::vector<std::pair<u64, int>> phi;
    u64 ph = 1;
    auto add = [&](u64 q, int e) {
        for (auto& [r, c] : phi)
            if (r == q) return void(c += e);
        phi.emplace_back(q, e);
    };
    const auto fm = factorize_pairs(m);
    cyclic = fm.size() == 1 ? fm[0].first != 2 || fm[0].second <= 2 : fm.size() == 2 && fm[0].first == 2 && fm[0].second == 1;
    for (auto [p, e] : fm) {
        for (int i = 1; i < e; ++i) ph *= p;
        if (e > 1) add(p, e - 1);
        ph *= p - 1;
        for (auto [q, f] : factorize_pairs(p - 1)) add(q, f);
    }
    // x の位数 n
    u64 n = ph;
    for (auto& [r, c] : phi)
        while (c > 0 && M.pow(x, n / r) == 1) n /= r, --c;
    if (M.pow(t, n) != 1) return -1;
    // n の素因数 r^e ごとに x mod r^e を求めて CRT でまとめる
    u64 X = 0, mod = 1;
    for (auto [r, e] : phi) {
        if (e == 0) continue;
        u64 re = 1;
        for (int i = 0; i < e; ++i) re *= r;
        const u64 gam = M.pow(x, n / re), tau = M.pow(t, n / re);
        const u64 zeta = M.pow(gam, re / r);
        if (cyclic && r > RHO_MIN) {
            // r^2 > 10^9 ≥ n なので e = 1、桁は 1 つ
            const u64 xr = rho_log(M, zeta, tau, r);
            const u64 k = (u64)((u128)((xr + re - X % re) % re) * ::inv_mod<u64>(mod % re, re) % re);
            X += mod * k, mod *= re;
            continue;
        }
        const OrderRLog<HK, BSH> lg(M, zeta, r);
        const u64 ginv = M.pow(gam, re - 1);
        u64 xr = 0, ri = 1, ginv_ri = ginv;  // ginv_ri = γ^{-r^i}
        u64 cur = tau;                       // τ γ^{-xr}
        for (int i = 0; i < e; ++i) {
            const long long l = lg(M.pow(cur, re / ri / r));
            if (l < 0) return -1;
            if (l) cur = M.mul(cur, M.pow(ginv_ri, (u64)l)), xr += (u64)l * ri;
            ginv_ri = M.pow(ginv_ri, r), ri *= r;
        }
        // X ≡ X (mod mod)、≡ xr (mod re) をまとめる
        const u64 k = (u64)((u128)((xr + re - X % re) % re) * ::inv_mod<u64>(mod % re, re) % re);
        X += mod * k, mod *= re;
    }
    return (i64)X;
}

template <int HK, int BSH, u64 RHO_MIN>
inline i64 discrete_log(u64 X, u64 Y, u64 M) {
    if (M == 1) return 0;
    // K ≤ 32 は直に調べる
    {
        u64 v = 1 % M;
        for (int k = 0; k <= 32; ++k, v = v * X % M)
            if (v == Y) return k;
    }
    u64 m = M, b = Y, d = 1;
    int c = 0;
    for (u64 g; (g = std::gcd(X, m)) > 1;) {
        if (b % g) return -1;
        m /= g, b /= g, ++c;
        d = d * (X / g) % m;
    }
    if (m == 1) return -1;  // K > 32 の解は K ≤ 32 の解より大きいので、ここに来るのは解が無いとき
    const u64 t = (u64)((u128)b * ::inv_mod<u64>(d % m, m) % m);
    const i64 k = coprime_log<HK, BSH, RHO_MIN>(X % m, t, m);
    return k < 0 ? -1 : k + c;
}

}  // namespace dl5
