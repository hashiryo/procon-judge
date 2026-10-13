#pragma once
// 法が固定の離散対数の 1 回目。index_calculus.hpp (yosupo の purplesyringa の提出の写し、maspy の 1 問 O(1) の方法) と
// 同じ手順で、次を替える。
// - a を Farey の表の区画に割り当てる floor(a NB / p) を、floor(2^64 NB / p) との掛け算の上位 64 bit で求める。誤差は
//   a 2^{-64} 未満で、a NB / p の小数部は 0 < a < p なら 1/p 以上なので、割り算と一致する。
// - p mod i の段 (i > √p で log i = log(p mod i) + log(-1) - log(p / i)) を、p / i が i ごとに高々 1 しか減らないことを
//   使って割り算なしで進める。
// - g^{-1} の掛け算と Pohlig-Hellman の累乗を Montgomery にする。mod (p - 1) の足し算は符号で補正する。
// - log の表と Farey の表を 2 MB 境界の mmap に置いて huge page を頼む (Linux)。答えの列も reserve してから頼み、0 で
//   埋めてから添字で書く。
//
// 以下は元の手順。a ≡ num / den (mod p) で |num| ≤ LIM、den < 2^16 となる分数を、a / p を NB 個に区切った区画ごとに
// Farey の表 (Stern-Brocot の中間数で埋める) で引き、log a = log |num| + [num < 0] (p-1)/2 - log den とする。log の表は
// LIM までの整数について前もって作る。最初の K1 個の素数の log は p - 1 の素因数ごとの Pohlig-Hellman (位数 r の
// baby-step giant-step を K1 個まとめて引く) で求め、√p までの残りの素数は、pp g^{-t} を Farey の表で分数にして分子と
// 分母の log が分かるまで t を進める。SM 以下の素数の冪を掛けた smooth な数の log も広げておく。√p までの合成数は最小の
// 素因数で、√p より大きい整数は p mod i の段で埋める。p < M0 なら g の冪を順にたどって全部の log を表にする。
#include "../common.hpp"
#ifdef __linux__
#include <sys/mman.h>
#endif

namespace dlf1 {
using i32 = int32_t;

// 0 で埋めた bytes の領域。2 MB 以上なら Linux では 2 MB 境界の mmap に置いて huge page を頼む。
class Zeroed {
    void* p = nullptr;
#ifdef __linux__
    size_t mapped = 0;
#endif
   public:
    explicit Zeroed(size_t bytes) {
#ifdef __linux__
        constexpr size_t H = size_t(1) << 21;
        if (bytes >= H) {
            const size_t b = (bytes + 4095) & ~size_t(4095);
            void* m = mmap(nullptr, b + H, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
            if (m != MAP_FAILED) {
                char *q = static_cast<char*>(m), *a = reinterpret_cast<char*>((reinterpret_cast<uintptr_t>(q) + H - 1) & ~(H - 1));
                if (a != q) munmap(q, a - q);
                munmap(a + b, q + H - a);
                madvise(a, b, MADV_HUGEPAGE);
                p = a, mapped = b;
                return;
            }
        }
#endif
        p = calloc(bytes ? bytes : 1, 1);
    }
    ~Zeroed() {
#ifdef __linux__
        if (mapped) {
            munmap(p, mapped);
            return;
        }
#endif
        free(p);
    }
    Zeroed(const Zeroed&) = delete;
    Zeroed& operator=(const Zeroed&) = delete;
    void* get() const { return p; }
};
template <class T>
inline void reserve_huge(std::vector<T>& v, size_t n) {
    v.reserve(n);
#ifdef __linux__
    const uintptr_t b = reinterpret_cast<uintptr_t>(v.data()) & ~uintptr_t(4095), e = (reinterpret_cast<uintptr_t>(v.data() + n) + 4095) & ~uintptr_t(4095);
    if (b < e) madvise(reinterpret_cast<void*>(b), e - b, MADV_HUGEPAGE);
#endif
}

// R = 2^32 の Montgomery。値は [0, 2p) のまま持つ (p < 2^30)
struct Mont {
    u32 p, pinv, r1, r2;
    explicit Mont(u32 p_) : p(p_) {
        u32 x = (3 * p) ^ 2;
        for (int i = 0; i < 3; ++i) x *= 2 - p * x;
        pinv = x, r1 = (0u - p) % p, r2 = (u32)((u64)r1 * r1 % p);
    }
    u32 mul(u32 a, u32 b) const {
        const u64 t = (u64)a * b;
        return (u32)(t >> 32) + p - (u32)((u64)((u32)t * pinv) * p >> 32);
    }
    u32 to(u32 a) const { return mul(a, r2); }
    u32 norm(u32 a) const { return a >= p ? a - p : a; }
    u32 from(u32 a) const { return norm(mul(a, 1)); }
    u32 pow(u32 b, u64 e) const {  // b は Montgomery 形
        u32 r = r1;
        for (; e; e >>= 1, b = mul(b, b))
            if (e & 1) r = mul(r, b);
        return r;
    }
    u32 powi(u32 b, u64 e) const { return from(pow(to(b), e)); }  // 普通の形で
};

inline u32 inv_mod(u32 a, u32 m) {
    i64 x = 1, y = 0;
    u32 b = m;
    while (b) {
        const u32 t = a / b;
        a -= t * b, std::swap(a, b);
        x -= (i64)t * y, std::swap(x, y);
    }
    return (u32)(x < 0 ? x + m : x);
}

constexpr u32 M0 = 2000000, NB = 1000000, LIM = 1300000, SQ = 31624, K1 = 25, SM = 100;

// 位数 n の元 z の離散対数を引く baby-step giant-step。表は z^j (j < B)、鍵は Montgomery 形を [0, p) に直した値
struct Bsgs {
    const Mont& M;
    u32 n, B, mask, step;  // step = z^{-B} (Montgomery 形)
    std::vector<u32> key, val;
    Bsgs(const Mont& M_, u32 z, u32 n_, u32 B_) : M(M_), n(n_), B(std::min(B_, n_)) {
        u32 cap = 16;
        while (cap < B + B / 4 + 1) cap *= 2;
        mask = cap - 1;
        key.assign(cap, 0), val.resize(cap);
        const u32 zm = M.to(z);
        u32 x = M.r1;
        for (u32 j = 0; j < B; ++j, x = M.mul(x, zm)) {
            const u32 k = M.norm(x);
            u32 h = (k * 0x9E3779B1u) >> 8 & mask;
            while (key[h]) h = (h + 1) & mask;
            key[h] = k, val[h] = j;
        }
        step = M.pow(zm, n - B);  // z^{-B}
    }
    u32 look(u32 k) const {
        for (u32 h = (k * 0x9E3779B1u) >> 8 & mask; key[h]; h = (h + 1) & mask)
            if (key[h] == k) return val[h];
        return ~0u;
    }
    u32 solve(u32 c) const {  // c は普通の形、z^x = c の x
        u32 x = M.to(c), r = 0;
        for (u32 v;; r += B, x = M.mul(x, step))
            if ((v = look(M.norm(x))) != ~0u) return r + v;
    }
};

struct DlogFixed {
    u32 p, pm1, halfp;
    bool small;
    u64 bk;  // floor(2^64 NB / p)
    Zeroed dmem, fmem;
    u32 *dl, *fa;
    u32 addm(u32 a, u32 b) const {  // (a + b) mod (p - 1)、a, b < p - 1
        const u32 t = a + b - pm1;
        return t + (pm1 & (u32)((i32)t >> 31));
    }
    u32 negm(u32 a) const { return a ? pm1 - a : 0; }
    u32 bucket(u32 a) const { return (u32)((u128)a * bk >> 64); }
    DlogFixed(u32 p_, u32 g) : p(p_), pm1(p_ - 1), halfp(p_ / 2), small(p_ < M0), dmem(4 * (size_t)(small ? p_ : LIM + 1)), fmem(small ? 0 : 4 * (size_t)NB) {
        dl = (u32*)dmem.get(), fa = (u32*)fmem.get();
        const Mont M(p);
        if (small) {
            const u32 gm = M.to(g);
            u32 x = gm;
            for (u32 i = 1; i < pm1; ++i, x = M.mul(x, gm)) dl[M.from(x)] = i;
            return;
        }
        bk = (u64)(((u128)NB << 64) / p);
        // Farey の表。区画 [x, y) を f1 と f2 の間の分数で埋める (分数は分子 << 16 | 分母)
        auto rec = [&](auto& self, u32 f1, u32 f2, u32 x, u32 y) -> void {
            const u32 f3 = f1 + f2;
            u32 l = (u32)((((u64)p * (f3 >> 16) - LIM) * NB - 1) / ((u64)p * (f3 & 0xffff)) + 1);
            u32 r = (u32)((((u64)p * (f3 >> 16) + LIM) * NB) / ((u64)p * (f3 & 0xffff)));
            l = std::max(l, x), r = std::min(r, y);
            if (x < l) self(self, f1, f3, x, l);
            for (u32 i = l; i < r; ++i) fa[i] = f3;
            if (r < y) self(self, f3, f2, r, y);
        };
        {
            const u32 fx = (u32)((u64)LIM * NB / p), fy = (u32)(((u64)(p - LIM) * NB - 1) / (p * 2ull) + 1);
            for (u32 i = 0; i < fx; ++i) fa[i] = 1;
            rec(rec, 1u, 0x10002u, fx, fy);
            for (u32 i = fy; i < NB / 2; ++i) fa[i] = 0x10002u;
            for (u32 i = NB / 2; i < NB; ++i) fa[i] = (fa[NB - 1 - i] * 0xffff0001u ^ 0xffff0000u) + 0x10000u;
        }
        // SQ までの最小の素因数
        std::vector<u32> spf(SQ + 1, 0), primes;
        primes.reserve(3500);
        for (u32 i = 2; i <= SQ; ++i) {
            if (!spf[i]) spf[i] = i, primes.push_back(i);
            for (u32 q : primes) {
                if (q * i > SQ) break;
                spf[q * i] = q;
                if (q == spf[i]) break;
            }
        }
        // 1 段目: 最初の K1 個の素数の log を Pohlig-Hellman で
        {
            std::vector<std::pair<u32, u32>> fac;
            u32 m = pm1;
            for (u32 q : primes) {
                if ((u64)q * q > m) break;
                if (m % q == 0) {
                    u32 e = 0;
                    do ++e, m /= q;
                    while (m % q == 0);
                    fac.emplace_back(q, e);
                }
            }
            if (m > 1) fac.emplace_back(m, 1);
            const u32 ginv = inv_mod(g, p);
            std::array<u32, K1> ans{};
            u32 mod = 1;
            for (auto [q, e] : fac) {
                u32 qe = 1;
                for (u32 i = 0; i < e; ++i) qe *= q;
                const u32 ee = pm1 / qe;
                const u32 gg = M.powi(ginv, ee);  // 位数 qe の部分群の生成元の逆元
                const Bsgs bs(M, M.powi(g, pm1 / q), q, q < 100 ? q : (u32)std::sqrt((double)q * K1));
                const u32 inv_mod_q = inv_mod(mod % qe, qe);
                for (u32 i = 0; i < K1; ++i) {
                    const u32 h = M.powi(primes[i], ee);
                    u32 pe = qe, x = 0, mul = 1;
                    for (u32 j = 0; j < e; ++j) {
                        pe /= q;
                        const u32 t = (u32)((u64)h * M.powi(gg, x) % p);
                        x += bs.solve(M.powi(t, pe)) * mul;
                        mul *= q;
                    }
                    // ans[i] ≡ ans[i] (mod mod)、≡ x (mod qe) をまとめる
                    const u32 k = (u32)((u64)((x + qe - ans[i] % qe) % qe) * inv_mod_q % qe);
                    ans[i] += mod * k;
                }
                mod *= qe;
            }
            for (u32 i = 0; i < K1; ++i) dl[primes[i]] = ans[i];
        }
        // 2 段目: SQ までの残りの素数と、SM 以下の素数の冪を掛けた smooth な数
        {
            const u32 ginvm = M.to(inv_mod(g, p));
            std::vector<u32> smooth{1};
            smooth.reserve(5000);
            for (u32 q : primes) {
                if (!dl[q]) {
                    u32 h = q, t = 0;
                    for (;; ++t, h = M.norm(M.mul(h, ginvm))) {  // h g^{-1} (h は普通の形、ginvm は Montgomery 形)
                        const u32 f = fa[bucket(h)], den = f & 0xffff;
                        const i32 num = (i32)(den * h - (f >> 16) * p);
                        const u32 an = (u32)std::abs(num);
                        if ((den != 1 && !dl[den]) || (an != 1 && !dl[an])) continue;
                        dl[q] = addm(addm(addm(dl[an], num < 0 ? halfp : 0), negm(dl[den])), t % pm1);
                        break;
                    }
                }
                if (q > SM) continue;
                smooth.erase(std::remove_if(smooth.begin(), smooth.end(), [&](u32 v) { return v > LIM / q; }), smooth.end());
                const size_t nn = smooth.size();
                for (size_t j = 0; j < nn; ++j) {
                    u32 prev = smooth[j], ns = prev * q;
                    do {
                        if (ns <= LIM / q) smooth.push_back(ns);
                        dl[ns] = addm(dl[prev], dl[q]);
                        prev = ns, ns *= q;
                    } while (prev <= LIM / q);
                }
            }
        }
        // 3 段目: SQ までの合成数
        for (u32 i = 2; i <= SQ; ++i)
            if (!dl[i]) dl[i] = addm(dl[i / spf[i]], dl[spf[i]]);
        // 5 段目: SQ より大きい i は p = q i + r (r = p mod i < i、q = p / i < i) から log i = log r + log(-1) - log q。
        // i が 1 増えると q は高々 1 減る (i (i + 1) > p)
        {
            u32 i = SQ + 1, q = p / i, r = p - q * i;
            for (; i <= LIM; ++i) {
                if (!dl[i]) dl[i] = addm(addm(dl[r], halfp), negm(dl[q]));
                if (r >= q) r -= q;
                else r += i + 1 - q, --q;
            }
        }
    }
    u32 operator()(u32 a) const {
        if (a == 0) return ~0u;
        if (small || a <= LIM) return dl[a];
        const u32 f = fa[bucket(a)], den = f & 0xffff;
        const i32 num = (i32)(den * a - (f >> 16) * p);
        const u32 s = (u32)(num >> 31), an = ((u32)num ^ s) - s;
        u32 r = dl[an] + (halfp & s) + pm1 - dl[den];  // < 3 (p - 1)
        u32 t = r - pm1;
        r = t + (pm1 & (u32)((i32)t >> 31));
        t = r - pm1;
        return t + (pm1 & (u32)((i32)t >> 31));
    }
};

}  // namespace dlf1
