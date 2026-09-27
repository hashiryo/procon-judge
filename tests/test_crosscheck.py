"""pj testdata crosscheck。参照実装を愚直解と小さい入力で突き合わせる。"""

import dataclasses
import io
from pathlib import Path

import pytest

from pj import cli
from pj import crosscheck as crosscheck_mod
from pj import environment as env_mod
from tests.test_local import make

# 和の 2 倍を、参照実装とは別の書き方で出す。
BRUTE = """#include <iostream>
int main() {
  int n; std::cin >> n;
  long long s = 0;
  while (n--) { long long x; std::cin >> x; s += x + x; }
  std::cout << s << "\\n";
}
"""
# n = 3 のときだけ 1 ずれる。
WRONG = BRUTE.replace('std::cout << s', 'std::cout << s + (s == 6)')
CRASH = "int main() { return 3; }\n"


@pytest.fixture(autouse=True)
def cache_in_tmp(tmp_path, monkeypatch):
    monkeypatch.setattr(crosscheck_mod, "CROSSCHECK_CACHE_DIR", tmp_path / "crosscheck")


def run(problem, brute_text, seeds=range(5)):
    (problem.dir / "brute.cpp").write_text(brute_text)
    out = io.StringIO()
    code = crosscheck_mod.crosscheck(
        problem, Path("brute.cpp"), env_mod.load("local"), seeds=seeds, out=out
    )
    return code, out.getvalue()


def test_agreeing_outputs_pass(tmp_path):
    code, out = run(make(tmp_path), BRUTE)
    assert code == 0, out
    assert "5 ケース中 5 件一致 / 食い違い 0 件 / 落ちた 0 件" in out


def test_a_mismatch_names_the_seed(tmp_path):
    # seed 2 の入力は 0 1 2 で、和の 2 倍が 6 になる。
    code, out = run(make(tmp_path), WRONG)
    assert code == 1
    assert "seed_0002: 食い違い" in out
    assert "食い違い 1 件" in out


def test_a_crash_is_counted(tmp_path):
    code, out = run(make(tmp_path), CRASH, seeds=range(2))
    assert code == 1
    assert "brute が落ちました" in out
    assert "落ちた 2 件" in out


def test_only_local_problems_can_be_checked(tmp_path):
    problem = make(tmp_path)
    manual = dataclasses.replace(problem, testdata=dataclasses.replace(problem.testdata, source="manual"))
    with pytest.raises(crosscheck_mod.CrosscheckError, match="local"):
        run(manual, BRUTE)


def test_seeds_are_a_half_open_range():
    assert crosscheck_mod.parse_seeds("1000:1003") == range(1000, 1003)
    assert crosscheck_mod.parse_seeds("7") == range(7, 8)
    with pytest.raises(crosscheck_mod.CrosscheckError):
        crosscheck_mod.parse_seeds("5:5")
    with pytest.raises(crosscheck_mod.CrosscheckError):
        crosscheck_mod.parse_seeds("a:b")


def test_cli_runs_the_crosscheck(tmp_path, monkeypatch, capsys):
    problem = make(tmp_path)
    (problem.dir / "brute.cpp").write_text(BRUTE)
    monkeypatch.setattr(cli.problem_mod, "load_by_id", lambda pid: problem)
    assert cli.main([
        "testdata", "crosscheck", "--problem", problem.id, "--brute", "brute.cpp", "--seeds", "0:3",
    ]) == 0
    assert "3 ケース中 3 件一致" in capsys.readouterr().out
