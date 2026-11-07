#pragma once
// 参照実装。遷移を std::map で持つ素直な接尾辞オートマトン。文字を足すと、異なる部分文字列の数は len[cur] - len[link[cur]] だけ増える
// (複製の状態を作っても数は変わらない)。
#include <map>

struct Solver {
  struct St {
    int len, link;
    map<char, int> next;
  };
  vector<St> st;
  int last = 0;
  u64 total = 0;
  Solver() : st(1, St{0, -1, {}}) {}
  u64 push(char c) {
    const int cur = (int)st.size();
    st.push_back(St{st[last].len + 1, 0, {}});
    int p = last;
    while (p != -1 && !st[p].next.count(c)) st[p].next[c] = cur, p = st[p].link;
    if (p != -1) {
      const int q = st[p].next[c];
      if (st[p].len + 1 == st[q].len) {
        st[cur].link = q;
      } else {
        const int cl = (int)st.size();
        St clone = st[q];
        clone.len = st[p].len + 1;
        st.push_back(clone);
        while (p != -1 && st[p].next[c] == q) st[p].next[c] = cl, p = st[p].link;
        st[q].link = st[cur].link = cl;
      }
    }
    last = cur;
    total += st[cur].len - st[st[cur].link].len;
    return total;
  }
};
