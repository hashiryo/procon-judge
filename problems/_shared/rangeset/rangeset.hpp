#pragma once
// 閉区間 [l, r] の集合。重なる区間は併合し、merge が true なら隣接する区間も併合する。
// 区間は左端を鍵にした map に、重ならない形で持つ。今の Library の RangeSet (std::set に
// 前後の番兵を入れる形) と書き比べるための候補。
//
// insert は新しく覆われた点の数、erase は覆われなくなった点の数を返す。
// covered_by は点 (または区間) を含む区間を返し、無ければ l > r の空の区間を返す。
#include <cstddef>
#include <iterator>
#include <limits>
#include <map>
#include <memory_resource>

template <class Int, bool merge = true> class RangeSet {
 public:
  struct Section {
    Int l, r;
    Int length() const { return r - l + 1; }
    operator bool() const { return l <= r; }
  };

 private:
  static constexpr Int GAP = merge ? 1 : 0;
  std::pmr::unsynchronized_pool_resource pool_;  // ノードの確保をプールから行う
  std::pmr::map<Int, Int> mp_;                    // 区間の左端 -> 右端

  static Section empty_section() { return {1, 0}; }
  static Int sub(Int a, Int b) {  // a - b。あふれないように下限で止める
    constexpr Int lo = std::numeric_limits<Int>::min();
    return a < lo + b ? lo : a - b;
  }
  static Int add(Int a, Int b) {
    constexpr Int hi = std::numeric_limits<Int>::max();
    return a > hi - b ? hi : a + b;
  }

 public:
  RangeSet() : mp_(&pool_) {}

  size_t size() const { return mp_.size(); }

  // [l, r] を含む区間。無ければ空。
  Section covered_by(Int l, Int r) const {
    auto it = mp_.upper_bound(l);
    if (it == mp_.begin()) return empty_section();
    --it;
    if (it->second < l || r > it->second) return empty_section();
    return {it->first, it->second};
  }
  Section covered_by(Int x) const { return covered_by(x, x); }
  Section covered_by(const Section &s) const { return covered_by(s.l, s.r); }

  // [l, r] を入れて、新しく覆われた点の数を返す。
  Int insert(Int l, Int r) {
    if (r < l) return 0;
    // 既に覆われているなら、消して入れ直さずに返す。
    auto up = mp_.upper_bound(l);
    if (up != mp_.begin()) {
      auto p = std::prev(up);
      if (p->second >= r) return 0;
      // 1 つの区間に右から触れて伸ばすだけなら、値を書き換える (鍵は変えずに済む)。
      if (p->second >= sub(l, GAP)) {
        auto nx = std::next(p);
        if (nx == mp_.end() || nx->first > add(r, GAP)) {
          const Int add_len = r - p->second;
          p->second = r;
          return add_len;
        }
      }
    }
    auto it = mp_.lower_bound(sub(l, GAP));
    if (it != mp_.begin()) {
      auto p = std::prev(it);
      if (p->second >= sub(l, GAP)) it = p;
    }
    Int nl = l, nr = r, covered = 0;
    while (it != mp_.end() && it->first <= add(r, GAP)) {
      if (it->first < nl) nl = it->first;
      if (it->second > nr) nr = it->second;
      covered += it->second - it->first + 1;
      it = mp_.erase(it);
    }
    mp_.emplace(nl, nr);
    return nr - nl + 1 - covered;
  }
  Int insert(Int x) { return insert(x, x); }
  Int insert(const Section &s) { return insert(s.l, s.r); }

  // [l, r] を消して、覆われなくなった点の数を返す。
  Int erase(Int l, Int r) {
    if (r < l) return 0;
    auto it = mp_.upper_bound(l);
    if (it != mp_.begin()) {
      auto p = std::prev(it);
      if (p->second >= l) it = p;
    }
    Int removed = 0;
    while (it != mp_.end() && it->first <= r) {
      const Int a = it->first, b = it->second;
      it = mp_.erase(it);
      const Int lo = a > l ? a : l, hi = b < r ? b : r;
      if (lo <= hi) removed += hi - lo + 1;
      if (a < l) mp_.emplace(a, sub(l, 1));
      if (b > r) mp_.emplace(add(r, 1), b);
    }
    return removed;
  }

  struct const_iterator {
    using iterator_category = std::forward_iterator_tag;
    using value_type = Section;
    using difference_type = std::ptrdiff_t;
    using pointer = const Section *;
    using reference = Section;
    std::pmr::map<Int, Int>::const_iterator it;
    Section operator*() const { return {it->first, it->second}; }
    const_iterator &operator++() {
      ++it;
      return *this;
    }
    const_iterator operator++(int) {
      const_iterator t = *this;
      ++it;
      return t;
    }
    bool operator==(const const_iterator &o) const { return it == o.it; }
    bool operator!=(const const_iterator &o) const { return it != o.it; }
  };
  const_iterator begin() const { return const_iterator{mp_.begin()}; }
  const_iterator end() const { return const_iterator{mp_.end()}; }
};
