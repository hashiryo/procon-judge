"""提出のソースの色付け。span を剥がすと元に戻ること、色の付く場所。"""

import html
import re

from pj.site.highlight import highlight

SOURCE = '''#pragma once
#include "common.hpp"  // shared
#include <vector>
// 説明
struct Solver {
  explicit Solver(const vector<i64> &a) : n((int)a.size()) {}
  i64 run() { return 1e9 + 7 + x; }
  const char *s = "// not a comment";
  bool ok = true;
};
'''


def strip(markup: str) -> str:
    return re.sub(r"</?span[^>]*>", "", markup)


def test_spans_wrap_the_escaped_source_exactly():
    assert strip(highlight(SOURCE)) == html.escape(SOURCE)


def test_keywords_strings_numbers_and_comments_are_colored():
    out = highlight(SOURCE)
    assert '<span class="hl-k">struct</span>' in out
    assert '<span class="hl-k">const</span>' in out
    assert '<span class="hl-s">&quot;// not a comment&quot;</span>' in out
    assert '<span class="hl-c">// 説明</span>' in out
    assert '<span class="hl-n">1e9</span>' in out
    assert '<span class="hl-n">true</span>' in out


def test_calls_and_definitions_are_function_colored():
    out = highlight(SOURCE)
    assert '<span class="hl-f">Solver</span>(' in out
    assert '<span class="hl-f">run</span>()' in out
    # 制御構文は関数ではない。
    assert '<span class="hl-k">return</span>' in out
    assert 'hl-f">if' not in highlight("if (x) y();")


def test_directives_split_head_path_and_comment():
    out = highlight('#include "common.hpp"  // shared\n#include <vector>\n#define F(x) x\n')
    assert '<span class="hl-p">#include</span> <span class="hl-s">&quot;common.hpp&quot;</span>  <span class="hl-c">// shared</span>' in out
    assert '<span class="hl-s">&lt;vector&gt;</span>' in out
    assert '<span class="hl-p">#define</span> F(x) x' in out


def test_braces_and_templates_stay_plain():
    """波括弧と山括弧は素のまま。数だけに色が付く。"""
    out = highlight("array<array<int, 1>, 1> x{{1}};")
    assert strip(out) == html.escape("array<array<int, 1>, 1> x{{1}};")
    assert out.count('<span class="hl-n">1</span>') == 3
    assert '<span class="hl-k">int</span>' in out
