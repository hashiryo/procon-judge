#pragma once
// Source: Library Checker "Convolution" (convolution_mod) の提出 406388 (Aiyiyi)、
//   https://judge.yosupo.jp/submission/406388
//   2026-11-25 の API の一覧で 0.023 s (AVX2、AVX-512 なし)。radix-4 pair のカーネル。
//   元のソースの先頭にあるとおり、I/O は QgQ さんの提出 393435 から取ったもの。
//   ライセンスの表示は無い。出典を残し、変更は最小限にする。
// 抽出方針: I/O (fastio) と main を削除し、カーネルをそのまま残して run(vector, vector) を足す。
//   カーネルは x86 のインライン asm (ymm の clobber) を含むので x64 だけで組める。
//   SIMDe では asm を読み替えられないため、arm は理由の分かるコンパイルエラーにする。
#include "../common.hpp"

#if !defined(__x86_64__)
#error "yosupo_406388_aiyiyi は x86-64 のインライン asm を含む。arm (SIMDe) では組めない。"
#endif

#if defined(__clang__)
#pragma clang attribute push(__attribute__((target("avx2"))), apply_to = function)
#define PJ_CLANG_TARGET_PUSHED 1
#else
#pragma GCC optimize("O3,unroll-loops")
#pragma GCC target("avx2,bmi")
#endif
#include <immintrin.h>

namespace kernel_asm_radix4_pair_large_fixed {
using U=uint32_t; using W=uint64_t; using V=__m256i;
constexpr U P=998244353, P2=2*P, NI=998244351, R2=932051910;
static_assert(W(4)*P < (W(1)<<32));
static_assert(W(8)*(P-1)*(P-1) < UINT64_MAX-W(UINT32_MAX)*P);
constexpr U muls(U a,U b) {
    W x=W(a)*b; U c=U(x)*NI; U z=(x+W(c)*P)>>32;
    return z>=P?z-P:z;
}
constexpr U power(U a,U e) { U r=1; for(;e;e>>=1,a=W(a)*a%P) if(e&1)r=W(r)*a%P;return r; }
constexpr U mont(U a) { return muls(a,R2); }
constexpr U ONE=mont(1);
inline V splat(U x) { return _mm256_set1_epi32(x); }
inline V plus(V x,V y) { return _mm256_add_epi32(x,y); }
inline V minus(V x,V y) { return _mm256_sub_epi32(x,y); }
inline V shrink(V x,U p) { return _mm256_min_epu32(x,minus(x,splat(p))); }
inline V low(V x) { return shrink(x,P2); }
inline V canonical(V x) { return shrink(low(x),P); }
inline V diff(V x,V y) { return minus(plus(x,splat(P2)),y); }
inline V odd(V x) { return _mm256_srli_epi64(x,32); }
// As in the user's Montgomery kernel, operate on even/odd lanes independently.
inline V reduce(V e,V o) {
    V m=splat(NI),p=splat(P);
    e=_mm256_add_epi64(e,_mm256_mul_epu32(_mm256_mul_epu32(e,m),p));
    o=_mm256_add_epi64(o,_mm256_mul_epu32(_mm256_mul_epu32(o,m),p));
    return _mm256_or_si256(odd(e),o);
}
struct Fixed {
    V w,wi;
    explicit Fixed(U x):w(splat(x)),wi(splat(x*NI)){}
    explicit Fixed(V x):w(x),wi(_mm256_mul_epu32(x,splat(NI))){}
    // x<4P, w<P => x*w + correction < 2^64 and result <2P.
    inline V operator()(V x) const {
        V e=_mm256_mul_epu32(x,w),o=_mm256_mul_epu32(odd(x),w);
        e=_mm256_add_epi64(e,_mm256_mul_epu32(_mm256_mul_epu32(x,wi),splat(P)));
        o=_mm256_add_epi64(o,_mm256_mul_epu32(_mm256_mul_epu32(odd(x),wi),splat(P)));
        return _mm256_or_si256(odd(e),o);
    }
};

// r[k] = product(q[b] for each set bit b of k), q[b]=3^((P-1)/2^(b+2)).
// Incrementing k clears t low bits and sets bit t; delta=q[t]/prod(q[0:t]).
// The radix-4 group uses x=r[k], y=r[2k], z=I*y (or x*y for twisted inputs).
struct Constants {
    U q[22]{},iq[22]{},step[22]{},istep[22]{},even_step[21]{},ieven_step[21]{};
    alignas(32) W rates[2][2][21][4]{};
    alignas(32) W fixed_rates[2][2][21][4]{};
    constexpr Constants() {
        q[21]=mont(power(3,(P-1)>>23)); iq[21]=mont(power(power(3,(P-1)>>23),P-2));
        for(int j=20;j>=0;--j) {q[j]=muls(q[j+1],q[j+1]);iq[j]=muls(iq[j+1],iq[j+1]);}
        U a=ONE,b=ONE,c=ONE,d=ONE;
        for(int j=0;j<22;++j) {
            step[j]=muls(q[j],a);istep[j]=muls(iq[j],b);
            a=muls(a,iq[j]);b=muls(b,q[j]);
            if(j<21) {
                even_step[j]=muls(q[j+1],c);ieven_step[j]=muls(iq[j+1],d);
                c=muls(c,iq[j+1]);d=muls(d,q[j+1]);
                for(int inverse=0;inverse<2;++inverse) for(int twist=0;twist<2;++twist) {
                    U x=inverse?istep[j]:step[j], y=inverse?ieven_step[j]:even_step[j];
                    rates[inverse][twist][j][0]=x; rates[inverse][twist][j][1]=y;
                    rates[inverse][twist][j][2]=twist?muls(x,y):y;rates[inverse][twist][j][3]=ONE;
                    for(int lane=0;lane<4;++lane) {
                        U value=U(rates[inverse][twist][j][lane]);
                        fixed_rates[inverse][twist][j][lane]=W(value)|(W(value*NI)<<32);
                    }
                }
            }
        }
    }
};
inline constexpr Constants constants{};
inline V packed_mul(V x,V y) {
    V t=_mm256_mul_epu32(x,y);
    V c=_mm256_mul_epu32(_mm256_mul_epu32(t,splat(NI)),splat(P));
    return shrink(_mm256_srli_epi64(_mm256_add_epi64(t,c),32),P);
}
inline V packed_mul_fixed(V x,V rate) {
    V product=_mm256_mul_epu32(x,rate);
    V correction=_mm256_mul_epu32(_mm256_mul_epu32(x,odd(rate)),splat(P));
    return shrink(_mm256_srli_epi64(_mm256_add_epi64(product,correction),32),P);
}
struct Twiddle { Fixed x,y,z; Twiddle(V a,V b,V c):x(a),y(b),z(c){} };

// Lazy=false is an ablation that keeps all butterfly values below 2P.
// Lazy=true: forward loads/stores <4P, inverse loads/stores <2P.
// Twist=true distributes w,w^2,w^3 onto inputs to expose independent multiplies.
// All four coefficients remain below 4P, exactly as the C++ formula.
// Twiddle has six consecutive aligned YMM members: x.w,x.wi,y.w,y.wi,z.w,z.wi.
__attribute__((always_inline)) inline void radix4_forward_asm(V* f,int h,const Twiddle& t) {
    static_assert(sizeof(Fixed)==64 && sizeof(Twiddle)==192);
    static constexpr U mod[2]={P,P2};
    V* a=f; V* b=f+h; V* c=f+2*h; V* d=f+3*h; int count=h;
    asm volatile(
        "vmovdqa 0(%[tw]), %%ymm4\n\t"
        "vmovdqa 32(%[tw]), %%ymm5\n\t"
        "vmovdqa 64(%[tw]), %%ymm6\n\t"
        "vmovdqa 96(%[tw]), %%ymm7\n\t"
        "vmovdqa 128(%[tw]), %%ymm8\n\t"
        "vmovdqa 160(%[tw]), %%ymm9\n\t"
        "vpbroadcastd 0(%[mod]), %%ymm10\n\t"
        "vpbroadcastd 4(%[mod]), %%ymm11\n\t"
        ".p2align 5\n\t"
        "1:\n\t"
        "vmovdqa (%[aa]), %%ymm0\n\t"
        "vmovdqa (%[bb]), %%ymm1\n\t"
        "vmovdqa (%[cc]), %%ymm2\n\t"
        "vmovdqa (%[dd]), %%ymm3\n\t"
        "vpsubd %%ymm11, %%ymm0, %%ymm12\n\t"
        "vpsubd %%ymm11, %%ymm1, %%ymm13\n\t"
        "vpminud %%ymm12, %%ymm0, %%ymm0\n\t"
        "vpminud %%ymm13, %%ymm1, %%ymm1\n\t"
        "vpsrlq $32, %%ymm2, %%ymm12\n\t"
        "vpsrlq $32, %%ymm3, %%ymm13\n\t"
        "vpmuludq %%ymm4, %%ymm2, %%ymm14\n\t"
        "vpmuludq %%ymm4, %%ymm3, %%ymm15\n\t"
        "vpmuludq %%ymm5, %%ymm2, %%ymm2\n\t"
        "vpmuludq %%ymm5, %%ymm3, %%ymm3\n\t"
        "vpmuludq %%ymm10, %%ymm2, %%ymm2\n\t"
        "vpmuludq %%ymm10, %%ymm3, %%ymm3\n\t"
        "vpaddq %%ymm2, %%ymm14, %%ymm14\n\t"
        "vpaddq %%ymm3, %%ymm15, %%ymm15\n\t"
        "vpmuludq %%ymm4, %%ymm12, %%ymm2\n\t"
        "vpmuludq %%ymm4, %%ymm13, %%ymm3\n\t"
        "vpmuludq %%ymm5, %%ymm12, %%ymm12\n\t"
        "vpmuludq %%ymm5, %%ymm13, %%ymm13\n\t"
        "vpmuludq %%ymm10, %%ymm12, %%ymm12\n\t"
        "vpmuludq %%ymm10, %%ymm13, %%ymm13\n\t"
        "vpaddq %%ymm12, %%ymm2, %%ymm2\n\t"
        "vpaddq %%ymm13, %%ymm3, %%ymm3\n\t"
        "vpsrlq $32, %%ymm14, %%ymm14\n\t"
        "vpsrlq $32, %%ymm15, %%ymm15\n\t"
        "vpor %%ymm14, %%ymm2, %%ymm2\n\t"
        "vpor %%ymm15, %%ymm3, %%ymm3\n\t"
        "vpaddd %%ymm2, %%ymm0, %%ymm12\n\t"
        "vpaddd %%ymm11, %%ymm0, %%ymm0\n\t"
        "vpsubd %%ymm2, %%ymm0, %%ymm0\n\t"
        "vpaddd %%ymm3, %%ymm1, %%ymm2\n\t"
        "vpaddd %%ymm11, %%ymm1, %%ymm1\n\t"
        "vpsubd %%ymm3, %%ymm1, %%ymm1\n\t"
        "vpsubd %%ymm11, %%ymm0, %%ymm14\n\t"
        "vpsubd %%ymm11, %%ymm12, %%ymm15\n\t"
        "vpminud %%ymm14, %%ymm0, %%ymm0\n\t"
        "vpminud %%ymm15, %%ymm12, %%ymm3\n\t"
        "vpsrlq $32, %%ymm2, %%ymm12\n\t"
        "vpsrlq $32, %%ymm1, %%ymm13\n\t"
        "vpmuludq %%ymm6, %%ymm2, %%ymm14\n\t"
        "vpmuludq %%ymm8, %%ymm1, %%ymm15\n\t"
        "vpmuludq %%ymm7, %%ymm2, %%ymm2\n\t"
        "vpmuludq %%ymm9, %%ymm1, %%ymm1\n\t"
        "vpmuludq %%ymm10, %%ymm2, %%ymm2\n\t"
        "vpmuludq %%ymm10, %%ymm1, %%ymm1\n\t"
        "vpaddq %%ymm2, %%ymm14, %%ymm14\n\t"
        "vpaddq %%ymm1, %%ymm15, %%ymm15\n\t"
        "vpmuludq %%ymm6, %%ymm12, %%ymm2\n\t"
        "vpmuludq %%ymm8, %%ymm13, %%ymm1\n\t"
        "vpmuludq %%ymm7, %%ymm12, %%ymm12\n\t"
        "vpmuludq %%ymm9, %%ymm13, %%ymm13\n\t"
        "vpmuludq %%ymm10, %%ymm12, %%ymm12\n\t"
        "vpmuludq %%ymm10, %%ymm13, %%ymm13\n\t"
        "vpaddq %%ymm12, %%ymm2, %%ymm2\n\t"
        "vpaddq %%ymm13, %%ymm1, %%ymm1\n\t"
        "vpsrlq $32, %%ymm14, %%ymm14\n\t"
        "vpsrlq $32, %%ymm15, %%ymm15\n\t"
        "vpor %%ymm14, %%ymm2, %%ymm2\n\t"
        "vpor %%ymm15, %%ymm1, %%ymm1\n\t"
        "vpaddd %%ymm2, %%ymm3, %%ymm12\n\t"
        "vpaddd %%ymm11, %%ymm3, %%ymm3\n\t"
        "vpsubd %%ymm2, %%ymm3, %%ymm3\n\t"
        "vpaddd %%ymm1, %%ymm0, %%ymm13\n\t"
        "vpaddd %%ymm11, %%ymm0, %%ymm0\n\t"
        "vpsubd %%ymm1, %%ymm0, %%ymm0\n\t"
        "vmovdqa %%ymm12, (%[aa])\n\t"
        "vmovdqa %%ymm3, (%[bb])\n\t"
        "vmovdqa %%ymm13, (%[cc])\n\t"
        "vmovdqa %%ymm0, (%[dd])\n\t"
        "add $32, %[aa]\n\t"
        "add $32, %[bb]\n\t"
        "add $32, %[cc]\n\t"
        "add $32, %[dd]\n\t"
        "dec %[count]\n\t"
        "jnz 1b\n\t"
        : [aa] "+&r"(a), [bb] "+&r"(b), [cc] "+&r"(c),
          [dd] "+&r"(d), [count] "+&r"(count)
        : [tw] "r"(&t), [mod] "r"(mod)
        : "cc", "memory", "ymm0", "ymm1", "ymm2", "ymm3", "ymm4", "ymm5", "ymm6", "ymm7", "ymm8", "ymm9", "ymm10", "ymm11", "ymm12", "ymm13", "ymm14", "ymm15");
}

template<bool Lazy,bool Twist,bool Identity,bool Inverse>
__attribute__((always_inline)) inline void radix4(V* f,int h,const Twiddle& t) {
    if constexpr(Lazy && !Twist && !Identity && !Inverse) {
        if(h>4){radix4_forward_asm(f,h,t);return;}
    }
    const Fixed imag(Inverse?constants.iq[0]:constants.q[0]);
    for(int j=0;j<h;++j) {
        V a=f[j],b=f[j+h],c=f[j+2*h],d=f[j+3*h];
        V o0,o1,o2,o3;
        if constexpr(!Inverse) {
            if constexpr(Lazy) a=low(a);
            if constexpr(Twist) {
                if constexpr(Identity) {if constexpr(Lazy) {b=low(b);c=low(c);d=low(d);}}
                else {b=t.y(b);c=t.x(c);d=t.z(d);}
                V ac=low(plus(a,c)), amc=low(diff(a,c));
                V bd=low(plus(b,d)), bmd=imag(Lazy?diff(b,d):low(diff(b,d)));
                o0=plus(ac,bd);o1=diff(ac,bd);o2=plus(amc,bmd);o3=diff(amc,bmd);
            } else {
                if constexpr(Lazy) b=low(b);
                if constexpr(Identity) {if constexpr(Lazy) {c=low(c);d=low(d);}}
                else {c=t.x(c);d=t.x(d);}
                V ac=low(plus(a,c)), amc=low(diff(a,c));
                V bd=plus(b,d), bmd=diff(b,d);
                if constexpr(!Lazy) {bd=low(bd);bmd=low(bmd);}
                if constexpr(Identity) bd=low(bd); else bd=t.y(bd);
                bmd=t.z(bmd);
                o0=plus(ac,bd);o1=diff(ac,bd);o2=plus(amc,bmd);o3=diff(amc,bmd);
            }
            if constexpr(!Lazy) {o0=low(o0);o1=low(o1);o2=low(o2);o3=low(o3);}
        } else {
            V ab=low(plus(a,b)),cd=low(plus(c,d));
            V amb=diff(a,b),cmd=diff(c,d);
            if constexpr(!Lazy) {amb=low(amb);cmd=low(cmd);}
            if constexpr(Twist) {
                amb=low(amb);cmd=imag(cmd);
                o0=low(plus(ab,cd));o1=plus(amb,cmd);o2=diff(ab,cd);o3=diff(amb,cmd);
                if constexpr(!Lazy) {o1=low(o1);o2=low(o2);o3=low(o3);}
                if constexpr(Identity) {o1=low(o1);o2=low(o2);o3=low(o3);}
                else {o1=t.y(o1);o2=t.x(o2);o3=t.z(o3);}
            } else {
                if constexpr(Identity) amb=low(amb);else amb=t.y(amb);
                cmd=t.z(cmd);
                o0=low(plus(ab,cd));o1=low(plus(amb,cmd));o2=diff(ab,cd);o3=diff(amb,cmd);
                if constexpr(!Lazy) {o2=low(o2);o3=low(o3);}
                if constexpr(Identity) {o2=low(o2);o3=low(o3);}else {o2=t.x(o2);o3=t.x(o3);}
            }
        }
        f[j]=o0;f[j+h]=o1;f[j+2*h]=o2;f[j+3*h]=o3;
    }
}

// Own direct8 method from exploration 002, with explicit incoming [0,4P) handling.
// Sum bound: 8*(P-1)^2+(2^32-1)*P <2^64. Reduce output <3P to <2P.
template<int Batch,int Schedule=0>
__attribute__((always_inline)) inline void leaf(V* a,V* b,const U* weights) {
    alignas(32) U window[Batch][16],coeff[Batch][8];
    V e[Batch],o[Batch];
    for(int t=0;t<Batch;++t) {
        V x=canonical(a[t]); Fixed w(weights[t]);
        _mm256_store_si256((V*)window[t],shrink(w(x),P));
        _mm256_store_si256((V*)(window[t]+8),x);
        _mm256_store_si256((V*)coeff[t],canonical(b[t]));
        e[t]=o[t]=_mm256_setzero_si256();
    }
    auto step = [&](int i) __attribute__((always_inline)) {
        for(int t=0;t<Batch;++t) {
            V x=_mm256_loadu_si256((V*)(window[t]+8-i)),y=splat(coeff[t][i]);
            e[t]=_mm256_add_epi64(e[t],_mm256_mul_epu32(x,y));
            o[t]=_mm256_add_epi64(o[t],_mm256_mul_epu32(odd(x),y));
        }
    };
    if constexpr(Schedule==1) {
        // The fully expanded leaf spilled many products in GCC 13 assembly.
        #pragma GCC unroll 1
        for(int i=0;i<8;++i)step(i);
    } else if constexpr(Schedule==2) {
        #pragma GCC unroll 2
        for(int i=0;i<8;++i)step(i);
    } else for(int i=0;i<8;++i)step(i);
    for(int t=0;t<Batch;++t)a[t]=low(reduce(e[t],o[t]));
}

// RootMode=0: O(N) root tables. RootMode=1: packed incremental per-stage cursors,
// and a scalar cursor for batches of four leaf factors. Tile is in AVX2 vectors.
template<bool Lazy,bool Twist,int RootMode,int Tile,int LeafBatch=4,int LeafSchedule=0,bool FuseTop=false>
struct Kernel {
    U *rt,*irt;
    V forward[12],inverse[12]; U leaf_cursor=ONE;
    Kernel(U* r,U* ir):rt(r),irt(ir) {
        for(int i=0;i<12;++i) {
            forward[i]=_mm256_setr_epi64x(ONE,ONE,Twist?ONE:constants.q[0],ONE);
            inverse[i]=_mm256_setr_epi64x(ONE,ONE,Twist?ONE:constants.iq[0],ONE);
        }
    }
    static void tables(int count,U* r,U* ir,int& size,bool fresh) {
        if(fresh||size==0) {r[0]=ir[0]=ONE;size=1;}
        for(int h=size;h<count;h*=2) {
            int s=__builtin_ctz(unsigned(h)); Fixed a(constants.q[s]),b(constants.iq[s]);
            if(h>=8) for(int j=0;j<h;j+=8) {
                _mm256_store_si256((V*)(r+h+j),shrink(a(_mm256_load_si256((V*)(r+j))),P));
                _mm256_store_si256((V*)(ir+h+j),shrink(b(_mm256_load_si256((V*)(ir+j))),P));
            } else for(int j=0;j<h;++j) {r[h+j]=muls(r[j],constants.q[s]);ir[h+j]=muls(ir[j],constants.iq[s]);}
        }
        size=std::max(size,count);
    }
    template<bool Inv>
    inline Twiddle twiddle(int h,int k) {
        if constexpr(RootMode==0 || RootMode==2) {
            const U* r=Inv?irt:rt;U x=r[k],y=r[2*k],z=Twist?muls(x,y):r[2*k+1];
            return Twiddle(splat(x),splat(y),splat(z));
        } else {
            int level=__builtin_ctz(unsigned(h))/2;
            V& cursor=Inv?inverse[level]:forward[level]; V current=cursor;
            int carry=__builtin_ctz(~unsigned(k));
            if constexpr(RootMode==3) {
                V step=_mm256_load_si256((const V*)constants.fixed_rates[Inv][Twist][carry]);
                cursor=packed_mul_fixed(current,step);
            } else {
                V step=_mm256_load_si256((const V*)constants.rates[Inv][Twist][carry]);
                cursor=packed_mul(current,step);
            }
            return Twiddle(_mm256_permute4x64_epi64(current,0x00),_mm256_permute4x64_epi64(current,0x55),_mm256_permute4x64_epi64(current,0xaa));
        }
    }
    template<bool Inv>
    __attribute__((always_inline)) inline void group(V* a,V* b,int h,int k) {
        if(h==1){
        Twiddle t=twiddle<Inv>(1,k);
        if(k==0) {
            radix4<Lazy,Twist,true,Inv>(a,1,t);
            if constexpr(!Inv)radix4<Lazy,Twist,true,false>(b,1,t);
        } else {
            radix4<Lazy,Twist,false,Inv>(a,1,t);
            if constexpr(!Inv)radix4<Lazy,Twist,false,false>(b,1,t);
        }

            return;
        }
        if(h==4){
        Twiddle t=twiddle<Inv>(4,k);
        if(k==0) {
            radix4<Lazy,Twist,true,Inv>(a,4,t);
            if constexpr(!Inv)radix4<Lazy,Twist,true,false>(b,4,t);
        } else {
            radix4<Lazy,Twist,false,Inv>(a,4,t);
            if constexpr(!Inv)radix4<Lazy,Twist,false,false>(b,4,t);
        }

            return;
        }
        Twiddle t=twiddle<Inv>(h,k);
        if(k==0) {
            radix4<Lazy,Twist,true,Inv>(a,h,t);
            if constexpr(!Inv)radix4<Lazy,Twist,true,false>(b,h,t);
        } else {
            radix4<Lazy,Twist,false,Inv>(a,h,t);
            if constexpr(!Inv)radix4<Lazy,Twist,false,false>(b,h,t);
        }
    }
    inline void leaves(V* a,V* b,int nv,int first) {
        for(int j=0;j<nv;j+=4) {
            group<false>(a+j,b+j,1,(first+j)/4);
            U w[4];
            if constexpr(RootMode==0)for(int t=0;t<4;++t)w[t]=muls(rt[first+j+t],rt[first+j+t]);
            else {
                w[0]=leaf_cursor;w[1]=P-w[0];w[2]=muls(w[0],constants.q[0]);w[3]=P-w[2];
                int carry=__builtin_ctz(~unsigned((first+j)/4));
                leaf_cursor=muls(leaf_cursor,constants.even_step[carry]);
            }
            if constexpr(LeafBatch==2) {leaf<2,LeafSchedule>(a+j,b+j,w);leaf<2,LeafSchedule>(a+j+2,b+j+2,w+2);}
            else leaf<4,LeafSchedule>(a+j,b+j,w);
            group<true>(a+j,nullptr,1,(first+j)/4);
        }
    }
    template<int NV,int H>
    __attribute__((always_inline)) inline void tile_forward(V* a,V* b,int k) {
        if constexpr(H>=4) {
            #pragma GCC unroll 1
            for(int j=0;j<NV;j+=4*H)group<false>(a+j,b+j,H,k*(NV/(4*H))+j/(4*H));
            tile_forward<NV,H/4>(a,b,k);
        }
    }
    template<int NV,int H>
    __attribute__((always_inline)) inline void tile_inverse(V* a,int k) {
        if constexpr(H<NV) {
            #pragma GCC unroll 1
            for(int j=0;j<NV;j+=4*H)group<true>(a+j,nullptr,H,k*(NV/(4*H))+j/(4*H));
            tile_inverse<NV,H*4>(a,k);
        }
    }
    template<int NV>
    __attribute__((noinline)) void fixed_tile(V* a,V* b,int k) {
        tile_forward<NV,NV/4>(a,b,k);
        leaves(a,b,NV,k*NV);
        tile_inverse<NV,4>(a,k);
    }
    void visit(V* a,V* b,int nv,int k) {
        if(nv<=Tile) {
            static_assert(Tile==256,"fixed tile experiment dispatches the original tile sizes");
            switch(nv) {
                case 4:fixed_tile<4>(a,b,k);break;
                case 16:fixed_tile<16>(a,b,k);break;
                case 64:fixed_tile<64>(a,b,k);break;
                case 256:fixed_tile<256>(a,b,k);break;
                default:assert(false);
            }
        } else {
            int h=nv/4;group<false>(a,b,h,k);
            for(int t=0;t<4;++t)visit(a+t*h,b+t*h,h,4*k+t);
            group<true>(a,nullptr,h,k);
        }
    }
    static void run(int n,U* aa,U* bb,U* r,U* ir,int& size,bool fresh) {
        assert(n>=64 && n<=(1<<22) && (n&(n-1))==0);
        if constexpr(RootMode==0 || RootMode==2)tables(n/(RootMode==2?16:8),r,ir,size,fresh);
        Kernel job(r,ir);V* a=(V*)aa;V* b=(V*)bb;int nv=n/8;
        if(__builtin_ctz(unsigned(nv))&1) {
            int h=nv/2;
            for(int i=0;i<h;++i) {
                V x=a[i],y=a[i+h];a[i]=low(plus(x,y));a[i+h]=low(diff(x,y));
                x=b[i];y=b[i+h];b[i]=low(plus(x,y));b[i+h]=low(diff(x,y));
            }
            job.visit(a,b,h,0);job.visit(a+h,b+h,h,1);
            if constexpr(FuseTop) {
                // The final sums/differences are <4P, acceptable to canonical scale.
                // This also removes both reduce2 operations and the normalization pass.
                Fixed scale(mont(mont(power(nv,P-2))));
                for(int i=0;i<h;++i) {V x=a[i],y=a[i+h];a[i]=shrink(scale(plus(x,y)),P);a[i+h]=shrink(scale(diff(x,y)),P);}
                return;
            } else for(int i=0;i<h;++i) {V x=a[i],y=a[i+h];a[i]=low(plus(x,y));a[i+h]=low(diff(x,y));}
        } else job.visit(a,b,nv,0);
        Fixed scale(mont(mont(power(nv,P-2))));
        for(int i=0;i<nv;++i)a[i]=shrink(scale(a[i]),P);
    }
};
}

namespace asm_radix4_pair_large_fixed {void invoke(int n,uint32_t*a,uint32_t*b,uint32_t*r,uint32_t*ir,int&s,bool fresh){kernel_asm_radix4_pair_large_fixed::Kernel<true,false,2,256,4,2,true>::run(n,a,b,r,ir,s,fresh);}}

namespace yosupo_406388_aiyiyi {
using u32= uint32_t;
// 元の提出の制限 (n, m <= 2^19) に合わせる。count <= 2^20 - 1 で length <= 2^20。
constexpr int max_transform= 1 << 20;
alignas(32) inline u32 fa[max_transform], fb[max_transform];
alignas(32) inline u32 roots[max_transform / 16], inverse_roots[max_transform / 16];

// yosupo の convolution_mod の run の口。c[k] = Σ_{i+j=k} a[i] b[j]。
inline std::vector<u32> run(const std::vector<u32>& x, const std::vector<u32>& y) {
 const int n= int(x.size()), m= int(y.size());
 if (!n || !m) return {};
 const int count= n + m - 1;
 int length= 64;
 while (length < count) length<<= 1;
 std::copy(x.begin(), x.end(), fa);
 std::fill(fa + n, fa + length, 0u);
 std::copy(y.begin(), y.end(), fb);
 std::fill(fb + m, fb + length, 0u);
 int root_size= 0;
 ::asm_radix4_pair_large_fixed::invoke(length, fa, fb, roots, inverse_roots, root_size, true);
 return std::vector<u32>(fa, fa + count);
}
}  // namespace yosupo_406388_aiyiyi

using yosupo_406388_aiyiyi::run;

#ifdef PJ_CLANG_TARGET_PUSHED
#undef PJ_CLANG_TARGET_PUSHED
#pragma clang attribute pop
#endif
