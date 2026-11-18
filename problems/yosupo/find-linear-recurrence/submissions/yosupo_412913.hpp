#pragma once
// Library Checker の find_linear_recurrence の最速の提出 (https://judge.yosupo.jp/submission/412913、sounansya) の
// Berlekamp–Massey の部分を写したもの。入出力と mint を除き、ModularField と restore はそのまま。
// ずれの内積はスカラーで u64 にためて 16 回ごとに剰余を取り、更新は Shoup の方法を AVX2 (CPU が持てば AVX-512) で行う。



#include <cmath>
#include <type_traits>

namespace field_detail {
template <class T>
struct Field {
	using V = T;
	static T add(T a, T b) { return a + b; }
	static T mul(T a, T b) { return a * b; }
	static T neg(T a) { return -a; }
	static T inv(T a) { return T(1) / a; }
	static bool zero(T a, long double eps) {
		if constexpr (std::is_floating_point_v<T>) return std::abs(a) <= eps;
		else { (void)eps; return a == T(0); }
	}
	static bool better(T a, T b) {
		if constexpr (std::is_floating_point_v<T>) return std::abs(a) > std::abs(b);
		else { (void)a; (void)b; return false; }
	}
	static T dot(const T *a, const T *b, int n) {
		T result = 0;
		for (int i = 0; i < n; ++i) result += a[i] * b[i];
		return result;
	}
	static void scale(T *a, int n, T x) { for (int i = 0; i < n; ++i) a[i] *= x; }
	static void axpy(T *a, const T *b, int n, T x) { for (int i = 0; i < n; ++i) a[i] += x * b[i]; }
};

}


#include <algorithm>
#include <cstdint>
#include <utility>
#if (defined(__x86_64__) || defined(__i386__)) && (defined(__GNUC__) || defined(__clang__))
#include <immintrin.h>
#endif

namespace field_detail {
template <class T, class = void>
struct SmallModint : std::false_type {};

template <class T>
struct SmallModint<T, std::void_t<decltype(std::integral_constant<std::uint32_t, T::mod()>{}),
	decltype(std::declval<T>().val()), decltype(T::raw(std::uint32_t{}))>>
	: std::bool_constant<(T::mod() > 1 && T::mod() < (std::uint32_t(1) << 31))
		&& std::is_convertible_v<decltype(std::declval<T>().val()), std::uint32_t>
		&& std::is_same_v<decltype(T::raw(std::uint32_t{})), T>> {};

template <class T>
inline constexpr bool small_modint = SmallModint<T>::value;

inline bool avx2() {
#if (defined(__x86_64__) || defined(__i386__)) && (defined(__GNUC__) || defined(__clang__)) && !defined(FIELD_DISABLE_AVX2) && !defined(MATRIX_DISABLE_AVX2)
	static const bool enabled = __builtin_cpu_supports("avx2");
	return enabled;
#else
	return false;
#endif
}

template <class T>
struct ModularField {
	using V = std::uint32_t;
	static constexpr V mod = T::mod();
	static V add(V a, V b) { V c = a + b; return c < mod ? c : c - mod; }
	static V mul(V a, V b) { return std::uint64_t(a) * b % mod; }
	static V neg(V a) { return a ? mod - a : 0; }
	static V dot(const V *a, const V *b, int n) {
		constexpr int step = mod <= (V(1) << 30) ? 16 : 4;
		std::uint64_t sum = 0;
		for (int i = 0; i < n; i += step) {
			for (int j = i; j < std::min(n, i + step); ++j) sum += std::uint64_t(a[j]) * b[j];
			sum %= mod;
		}
		return sum;
	}
	static V inv(V a) { return (T(1) / T::raw(a)).val(); }
	static bool zero(V a, long double) { return !a; }
	static bool better(V, V) { return false; }
	static V product(V a, V x, V q) {
		V r = a * x - V((std::uint64_t(a) * q) >> 32) * mod;
		return r < mod ? r : r - mod;
	}
#if (defined(__x86_64__) || defined(__i386__)) && (defined(__GNUC__) || defined(__clang__))
	__attribute__((target("avx2"))) static __m256i product8(__m256i a, __m256i x, __m256i q) {
		__m256i p = _mm256_set1_epi32(mod);
		__m256i even = _mm256_srli_epi64(_mm256_mul_epu32(a, q), 32);
		__m256i odd = _mm256_mul_epu32(_mm256_srli_epi64(a, 32), q);
		__m256i high = _mm256_or_si256(even, _mm256_slli_epi64(_mm256_srli_epi64(odd, 32), 32));
		__m256i r = _mm256_sub_epi32(_mm256_mullo_epi32(a, x), _mm256_mullo_epi32(high, p));
		return _mm256_min_epu32(r, _mm256_sub_epi32(r, p));
	}
	__attribute__((target("avx512f"))) static __m512i product16(__m512i a, __m512i x, __m512i q) {
		const auto p = _mm512_set1_epi32(mod);
		auto even = _mm512_srli_epi64(_mm512_mul_epu32(a, q), 32);
		auto odd = _mm512_mul_epu32(_mm512_srli_epi64(a, 32), q);
		auto high = _mm512_mask_blend_epi32(0xaaaa, even, odd);
		auto r = _mm512_sub_epi32(_mm512_mullo_epi32(a, x), _mm512_mullo_epi32(high, p));
		return _mm512_min_epu32(r, _mm512_sub_epi32(r, p));
	}
	template <bool add_to>
	__attribute__((target("avx512f"))) static void apply16(V *a, const V *b, int n, V x, V q) {
		const auto vx = _mm512_set1_epi32(x), vq = _mm512_set1_epi32(q), p = _mm512_set1_epi32(mod);
		int i = 0;
		for (; i + 16 <= n; i += 16) {
			auto r = product16(_mm512_loadu_si512(b + i), vx, vq);
			if constexpr (add_to) {
				r = _mm512_add_epi32(r, _mm512_loadu_si512(a + i));
				r = _mm512_min_epu32(r, _mm512_sub_epi32(r, p));
			}
			_mm512_storeu_si512(a + i, r);
		}
		for (; i < n; ++i) {
			V r = product(b[i], x, q);
			if constexpr (add_to) a[i] = add(a[i], r);
			else a[i] = r;
		}
	}
	template <bool add_to>
	__attribute__((target("avx2"))) static void apply8(V *a, const V *b, int n, V x, V q) {
		__m256i vx = _mm256_set1_epi32(x), vq = _mm256_set1_epi32(q), p = _mm256_set1_epi32(mod);
		int i = 0;
		for (; i + 8 <= n; i += 8) {
			__m256i r = product8(_mm256_loadu_si256(reinterpret_cast<const __m256i *>(b + i)), vx, vq);
			if constexpr (add_to) {
				r = _mm256_add_epi32(r, _mm256_loadu_si256(reinterpret_cast<const __m256i *>(a + i)));
				r = _mm256_min_epu32(r, _mm256_sub_epi32(r, p));
			}
			_mm256_storeu_si256(reinterpret_cast<__m256i *>(a + i), r);
		}
		for (; i < n; ++i) {
			V r = product(b[i], x, q);
			if constexpr (add_to) a[i] = add(a[i], r);
			else a[i] = r;
		}
	}
#endif
	template <bool add_to>
	static void apply(V *a, const V *b, int n, V x) {
		V q = (std::uint64_t(x) << 32) / mod;
#if (defined(__x86_64__) || defined(__i386__)) && (defined(__GNUC__) || defined(__clang__))
		if (n >= 64 && avx2() && __builtin_cpu_supports("avx512f")) { apply16<add_to>(a, b, n, x, q); return; }
		if (n >= 16 && avx2()) { apply8<add_to>(a, b, n, x, q); return; }
#endif
		for (int i = 0; i < n; ++i) {
			V r = product(b[i], x, q);
			if constexpr (add_to) a[i] = add(a[i], r);
			else a[i] = r;
		}
	}
	static void scale(V *a, int n, V x) { apply<false>(a, a, n, x); }
	static void axpy(V *a, const V *b, int n, V x) { apply<true>(a, b, n, x); }
};

}


namespace berlekamp_massey_detail {
template <class Ops>
std::vector<typename Ops::V> restore(const std::vector<typename Ops::V> &sequence) {
	using V = typename Ops::V;
	const int n = int(sequence.size());
	std::vector<V> reversed(sequence.rbegin(), sequence.rend()), current{V(1)}, previous{V(1)}, saved;
	current.reserve(sequence.size() + 1);
	previous.reserve(sequence.size() + 1);
	saved.reserve(sequence.size() + 1);
	int order = 0, shift = 1;
	V inverse = 1;
	for (int i = 0; i < n; ++i) {
		V discrepancy = Ops::dot(current.data(), reversed.data() + n - 1 - i, int(current.size()));
		if (Ops::zero(discrepancy, 0)) {
			++shift;
			continue;
		}
		bool grow = order <= i / 2;
		if (grow) saved.assign(current.begin(), current.end());
		V factor = Ops::neg(Ops::mul(discrepancy, inverse));
		if (current.size() < previous.size() + shift) current.resize(previous.size() + shift);
		Ops::axpy(current.data() + shift, previous.data(), int(previous.size()), factor);
		if (grow) {
			order = i + 1 - order;
			previous.swap(saved);
			inverse = Ops::inv(discrepancy);
			shift = 1;
		} else ++shift;
	}
	current.resize(std::size_t(order) + 1);
	current.erase(current.begin());
	for (auto &x : current) x = Ops::neg(x);
	return current;
}
}  // namespace berlekamp_massey_detail
struct yosupo_412913_mint {
 static constexpr std::uint32_t mod() { return 998244353; }
 std::uint32_t v;
 static yosupo_412913_mint raw(std::uint32_t x) { return {x}; }
 std::uint32_t val() const { return v; }
 friend yosupo_412913_mint operator/(yosupo_412913_mint a, yosupo_412913_mint b) {
  std::uint64_t r= 1, x= b.v;
  for(std::uint32_t e= mod() - 2; e; e>>= 1, x= x * x % mod())
   if(e & 1) r= r * x % mod();
  return {std::uint32_t(std::uint64_t(a.v) * r % mod())};
 }
 yosupo_412913_mint(std::uint32_t x= 0): v(x) {}
};
inline vector<u32> run(const vector<u32> &a) { return berlekamp_massey_detail::restore<field_detail::ModularField<yosupo_412913_mint>>(a); }
