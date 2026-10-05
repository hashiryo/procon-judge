#pragma once
// yosupo #393188 (isaunoya、2026-08-14) の移植。Library Checker の Convolution (F_{2^64}) で時間が一番短い提出 (0.409 s、入出力込み)。
// Taylor 展開で基底を替える Gao-Mateer 形の加法的 FFT を再帰で書いたもので、掛け算はスカラの clmul (x64 は PCLMULQDQ、arm は arm_neon の PMULL)。
// main を除き、入出力の型をハーネスに合わせて std::uint64_t を unsigned long long に替え、run から呼ぶ。ほかは元のまま。
#include <algorithm>
#include <cassert>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <random>
#include <span>
#include <vector>

/// @complexity Time: O((n + m) log(n + m)).
/// Space: O(n + m).

#if defined(__x86_64__)
#include <immintrin.h>
#elif defined(__aarch64__)
#include <arm_neon.h>
#endif

namespace noya {

namespace convolution_f2_64_internal {

using u64 = unsigned long long;
using u128 = unsigned __int128;

#if defined(__x86_64__)
__attribute__((target("pclmul"))) inline u128 carryless_multiply(u64 first,
                                                                 u64 second) {
  __m128i product = _mm_clmulepi64_si128(_mm_cvtsi64_si128(first),
                                         _mm_cvtsi64_si128(second), 0);
  u128 result;
  std::memcpy(&result, &product, sizeof(result));
  return result;
}
#elif defined(__aarch64__)
__attribute__((target("+crypto"))) inline u128 carryless_multiply(u64 first,
                                                                  u64 second) {
  poly128_t product = vmull_p64(poly64_t(first), poly64_t(second));
  u128 result;
  std::memcpy(&result, &product, sizeof(result));
  return result;
}
#else
inline u128 carryless_multiply(u64 first, u64 second) {
  u128 result = 0;
  for (int bit = 0; bit < 64; bit++) {
    if ((first >> bit) & 1) {
      result ^= u128(second) << bit;
    }
  }
  return result;
}
#endif

} // namespace convolution_f2_64_internal

/// @brief Element of GF(2^64) represented modulo
/// x^64+x^4+x^3+x+1. Addition is XOR; multiplication uses a carry-less
/// product and folds its high half twice through x^64=x^4+x^3+x+1.
class gf2_64 {
public:
  using value_type = unsigned long long;

  gf2_64() = default;
  explicit gf2_64(value_type value) : value_(value) {}

  value_type value() const { return value_; }

  gf2_64 &operator+=(gf2_64 other) {
    value_ ^= other.value_;
    return *this;
  }
  gf2_64 &operator-=(gf2_64 other) { return *this += other; }
  gf2_64 &operator*=(gf2_64 other) {
    using namespace convolution_f2_64_internal;
    u128 product = carryless_multiply(value_, other.value_);
    u128 once = u64(product) ^ carryless_multiply(u64(product >> 64), 0x1b);
    value_ = u64(once) ^ u64(carryless_multiply(u64(once >> 64), 0x1b));
    return *this;
  }

  friend gf2_64 operator+(gf2_64 first, gf2_64 second) {
    return first += second;
  }
  friend gf2_64 operator-(gf2_64 first, gf2_64 second) {
    return first -= second;
  }
  friend gf2_64 operator*(gf2_64 first, gf2_64 second) {
    return first *= second;
  }
  friend bool operator==(gf2_64, gf2_64) = default;

  gf2_64 power(value_type exponent) const {
    gf2_64 result(1);
    gf2_64 base = *this;
    while (exponent != 0) {
      if (exponent & 1) {
        result *= base;
      }
      base *= base;
      exponent >>= 1;
    }
    return result;
  }

  gf2_64 inverse() const {
    assert(value_ != 0);
    return power(~value_type(0) - 1);
  }

private:
  value_type value_ = 0;
};

namespace convolution_f2_64_internal {

using field = gf2_64;

inline std::vector<field> subset_sums(const std::vector<field> &basis) {
  std::vector<field> result(std::size_t(1) << basis.size());
  for (int bit = 0; bit < int(basis.size()); bit++) {
    for (int mask = 0; mask < (1 << bit); mask++) {
      result[(1 << bit) + mask] = result[mask] + basis[bit];
    }
  }
  return result;
}

struct additive_fft_data {
  std::vector<field> basis;
  std::vector<field> evaluation_offsets;
  std::vector<field> next_basis;
  std::vector<field> subset_evaluation_offsets;
  mutable std::vector<field> scratch;

  void initialize() {
    int logarithm = int(basis.size());
    scratch.resize(std::size_t(1) << logarithm);
    field last_inverse = basis.back().inverse();
    evaluation_offsets.resize(logarithm - 1);
    next_basis.resize(logarithm - 1);
    for (int index = 0; index + 1 < logarithm; index++) {
      evaluation_offsets[index] = basis[index] * last_inverse;
      next_basis[index] = evaluation_offsets[index] *
                              evaluation_offsets[index] +
                          evaluation_offsets[index];
    }
    subset_evaluation_offsets = subset_sums(evaluation_offsets);
  }
};

class additive_fft_cache {
public:
  void prepare(int logarithm) {
    if (int(data_.size()) > logarithm) {
      return;
    }
    std::mt19937_64 random;
    std::vector<field> chain;
    while (int(chain.size()) < logarithm) {
      chain.clear();
      for (field value(random()); value != field();
           value = value * value + value) {
        chain.push_back(value);
      }
    }
    chain.erase(chain.begin(), chain.end() - logarithm);
    data_.assign(logarithm + 1, additive_fft_data{});
    data_[logarithm].basis = std::move(chain);
    for (int level = logarithm; level > 0; level--) {
      data_[level].initialize();
      data_[level - 1].basis = data_[level].next_basis;
    }
  }

  const additive_fft_data &operator[](int logarithm) const {
    return data_[logarithm];
  }

private:
  std::vector<additive_fft_data> data_;
};

inline additive_fft_cache fft_cache;

template <bool Inverse> void taylor_transform(std::span<field> values) {
  if constexpr (Inverse) {
    for (std::size_t block = 1; block * 4 <= values.size(); block *= 2) {
      for (std::size_t start = 0; start < values.size(); start += block * 4) {
        for (std::size_t index = 0; index < block; index++) {
          field second = values[start + block + index];
          field third = values[start + block * 2 + index];
          field fourth = values[start + block * 3 + index];
          values[start + block + index] = second + third;
          values[start + block * 2 + index] = third + fourth;
        }
      }
    }
  } else {
    for (std::size_t block = values.size() / 4; block >= 1; block /= 2) {
      for (std::size_t start = 0; start < values.size(); start += block * 4) {
        for (std::size_t index = 0; index < block; index++) {
          field second = values[start + block + index];
          field third = values[start + block * 2 + index];
          field fourth = values[start + block * 3 + index];
          values[start + block + index] = second + third + fourth;
          values[start + block * 2 + index] = third + fourth;
        }
      }
    }
  }
}

template <bool Inverse = false> void additive_fft(std::span<field> values) {
  if (values.size() == 1) {
    return;
  }
  int logarithm = 63 - __builtin_clzll(values.size());
  const additive_fft_data &data = fft_cache[logarithm];
  if (values.size() == 2) {
    values[1] += values[0];
    return;
  }

  std::size_t half = values.size() / 2;
  std::span<field> even(data.scratch.data(), half);
  std::span<field> odd(data.scratch.data() + half, half);
  if constexpr (!Inverse) {
    taylor_transform<false>(values);
    for (std::size_t index = 0; index < half; index++) {
      even[index] = values[index * 2];
      odd[index] = values[index * 2 + 1];
    }
    additive_fft(even);
    additive_fft(odd);
    for (std::size_t index = 0; index < half; index++) {
      field first = even[index] +
                    data.subset_evaluation_offsets[index] * odd[index];
      values[index] = first;
      values[index + half] = first + odd[index];
    }
  } else {
    for (std::size_t index = 0; index < half; index++) {
      odd[index] = values[index] + values[index + half];
      even[index] = values[index] +
                    data.subset_evaluation_offsets[index] * odd[index];
    }
    additive_fft<true>(even);
    additive_fft<true>(odd);
    for (std::size_t index = 0; index < half; index++) {
      values[index * 2] = even[index];
      values[index * 2 + 1] = odd[index];
    }
    taylor_transform<true>(values);
  }
}

inline std::vector<field> naive_convolution(const std::vector<field> &first,
                                            const std::vector<field> &second) {
  if (first.empty() || second.empty()) {
    return {};
  }
  std::vector<field> result(first.size() + second.size() - 1);
  for (std::size_t i = 0; i < first.size(); i++) {
    for (std::size_t j = 0; j < second.size(); j++) {
      result[i + j] += first[i] * second[j];
    }
  }
  return result;
}

inline std::vector<field> convolve(std::vector<field> first,
                                   std::vector<field> second) {
  if (first.empty() || second.empty()) {
    return {};
  }
  std::size_t first_size = first.size();
  std::size_t second_size = second.size();
  int logarithm = 0;
  while ((std::size_t(1) << logarithm) < first_size + second_size - 1) {
    logarithm++;
  }
  std::size_t transform_size = std::size_t(1) << logarithm;
  if (first_size * second_size <=
      transform_size * std::size_t(logarithm + 1) * (logarithm + 1)) {
    return naive_convolution(first, second);
  }

  if (logarithm > 3 &&
      first_size + second_size - 1 ==
          (std::size_t(1) << (logarithm - 1)) + 1) {
    std::vector<field> tail(second_size);
    for (std::size_t index = 0; index < second_size; index++) {
      tail[index] = first.back() * second[index];
    }
    first.pop_back();
    std::vector<field> result = convolve(std::move(first), std::move(second));
    result.push_back(field());
    for (std::size_t index = 0; index < second_size; index++) {
      result[first_size - 1 + index] += tail[index];
    }
    return result;
  }

  fft_cache.prepare(logarithm);
  first.resize(transform_size);
  second.resize(transform_size);
  additive_fft(std::span<field>(first));
  additive_fft(std::span<field>(second));
  for (std::size_t index = 0; index < transform_size; index++) {
    first[index] *= second[index];
  }
  additive_fft<true>(std::span<field>(first));
  first.resize(first_size + second_size - 1);
  return first;
}

} // namespace convolution_f2_64_internal

/// @brief Convolution over GF(2^64). The transform evaluates in a tower basis
/// built from the Artin-Schreier map x -> x^2+x; its subspace polynomials split
/// each evaluation set into two affine halves, giving radix-two butterflies in
/// characteristic two without requiring roots of unity.
inline std::vector<unsigned long long>
convolution_f2_64(const std::vector<unsigned long long> &first,
                  const std::vector<unsigned long long> &second) {
  using namespace convolution_f2_64_internal;
  std::vector<field> converted_first;
  std::vector<field> converted_second;
  converted_first.reserve(first.size());
  converted_second.reserve(second.size());
  for (unsigned long long value : first) {
    converted_first.emplace_back(value);
  }
  for (unsigned long long value : second) {
    converted_second.emplace_back(value);
  }
  std::vector<field> converted_result =
      convolve(std::move(converted_first), std::move(converted_second));
  std::vector<unsigned long long> result(converted_result.size());
  for (std::size_t index = 0; index < result.size(); index++) {
    result[index] = converted_result[index].value();
  }
  return result;
}

} // namespace noya

inline std::vector<u64> run(int, int, const std::vector<u64>& a, const std::vector<u64>& b) { return noya::convolution_f2_64(a, b); }
