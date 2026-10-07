"""pj fallback。Library の実行時の分岐を、代わりの経路までエミュレーションで確かめる。"""

import signal
from pathlib import Path

import pytest

from pj import environment as env_mod
from pj import execute
from pj import fallback as fallback_mod
from pj import libraries as lib_mod
from pj import problem as problem_mod
from tests.test_run import EXIT_TOML, PICKY, RAW_TOML, make_case_problem

LIBRARIES = [lib_mod.Library(name="Library", prefix="mylib/", page="", source="")]

# エミュレータの代わり。バイナリをそのまま起こす。
PASS_THROUGH = ("sh", "-c", 'exec "$0"')


@pytest.fixture
def local_env():
    return env_mod.load("local")


def make_layout(tmp_path, toml=EXIT_TOML):
    """Library の代わりの lib/ と、提出を 3 本持つ問題。"""
    lib = tmp_path / "lib" / "mylib"
    lib.mkdir(parents=True)
    (lib / "fast.hpp").write_text(
        "#pragma once\n"
        "// vpclmulqdq を持つ CPU だけ速い経路を通る\n"
        'inline bool fast() { return __builtin_cpu_supports("vpclmulqdq"); }\n'
    )
    (lib / "plain.hpp").write_text(
        "#pragma once\n// AVX-512 も GFNI も使わない\ninline int plain() { return 1; }\n"
    )
    directory = tmp_path / "tmp-raw"
    (directory / "submissions").mkdir(parents=True)
    (directory / "problem.toml").write_text(toml)
    subs = directory / "submissions"
    (subs / "a.cpp").write_text('#include "mylib/fast.hpp"\nint main() { return fast() ? 0 : 0; }\n')
    (subs / "b.cpp").write_text('#include "mylib/plain.hpp"\nint main() { return plain() - 1; }\n')
    (subs / "c.cpp").write_text(
        '#include "mylib/fast.hpp"\n#include <immintrin.h>\n'
        "__m256i f(__m256i a) { return _mm256_clmulepi64_epi128(a, a, 0); }\n"
        "int main() { return 0; }\n"
    )
    return problem_mod.load(directory), [tmp_path / "lib", directory]


# --- 対象の選び方 --------------------------------------------------------------


def test_comments_do_not_count():
    assert not fallback_mod.MARKERS.search(fallback_mod.code("// vpclmulqdq\n/* gfni\n avx512 */ int x;"))
    assert fallback_mod.MARKERS.search(fallback_mod.code('__builtin_cpu_supports("avx2")'))


def test_scan_separates_library_headers_from_the_submission(tmp_path):
    problem, search = make_layout(tmp_path)
    scan = lambda name: fallback_mod.scan(problem, Path(f"submissions/{name}"), LIBRARIES, search=search)
    assert scan("a.cpp") == (("mylib/fast.hpp",), ())
    # 印の無いヘッダはコメントに命令の名前があっても対象にしない。
    assert scan("b.cpp") == ((), ())
    headers, own = scan("c.cpp")
    assert headers == ("mylib/fast.hpp",)
    assert len(own) == 1 and own[0].endswith("submissions/c.cpp")


def test_select_keeps_only_runnable_library_users(tmp_path, monkeypatch):
    problem, search = make_layout(tmp_path)
    monkeypatch.setattr(fallback_mod.build_mod, "include_dirs", lambda p: search)
    selection = fallback_mod.select([problem], LIBRARIES)
    assert [t.submission.as_posix() for t in selection.targets] == ["submissions/a.cpp"]
    assert [(e.submission.as_posix(), e.reason.split(" ")[0]) for e in selection.excluded] == [
        ("submissions/c.cpp", "提出の側にも印がある")
    ]


def test_select_excludes_problems_without_cases(tmp_path, monkeypatch):
    problem, search = make_layout(tmp_path, toml=RAW_TOML)
    monkeypatch.setattr(fallback_mod.build_mod, "include_dirs", lambda p: search)
    selection = fallback_mod.select([problem], LIBRARIES)
    assert selection.targets == ()
    assert {e.reason for e in selection.excluded} == {"走らせるケースが無い"}


# --- 判定 ----------------------------------------------------------------------


def result(**over):
    base = {"exit_code": 0, "term_signal": None, "timed_out": False, "wall_ns": 0, "max_rss_kb": 0}
    base.update(over)
    return execute.RunResult(**base)


def test_sigill_is_named(tmp_path):
    problem, _ = make_layout(tmp_path)
    status, detail = fallback_mod.judge(
        result(exit_code=None, term_signal=signal.SIGILL), problem, limit_sec=10
    )
    assert status == "RE" and "SIGILL" in detail and "CPU に無い命令" in detail


def test_memory_is_not_judged(tmp_path):
    """測れるのは QEMU のメモリなので、上限を超えていても MLE にしない。"""
    problem, _ = make_layout(tmp_path)
    assert fallback_mod.judge(result(max_rss_kb=10**9), problem, limit_sec=10)[0] == "AC"


def test_timeout_mentions_the_scale(tmp_path):
    problem, _ = make_layout(tmp_path)
    status, detail = fallback_mod.judge(
        result(timed_out=True, exit_code=None, term_signal=signal.SIGKILL),
        problem, limit_sec=problem.limits.tle_sec * 20,
    )
    assert status == "TLE" and "20 倍" in detail


# --- 走らせる ------------------------------------------------------------------


def test_run_goes_through_the_wrapper_without_the_preload(tmp_path):
    script = tmp_path / "prog.sh"
    script.write_text('#!/bin/sh\necho "ran ${LD_PRELOAD:-none}"\n')
    script.chmod(0o755)
    out = tmp_path / "out"
    wrapped = execute.run(
        script, stdin_path=None, stdout_path=out, stderr_path=tmp_path / "err", tle_sec=10,
        wrapper=("sh", "-c", 'echo wrapped; exec "$0"'),
    )
    assert wrapped.exit_code == 0
    lines = out.read_text().splitlines()
    assert lines[0] == "wrapped"
    assert lines[1].startswith("ran") and "rss_preload" not in lines[1]


def make_target(tmp_path, cases):
    problem, testcases = make_case_problem(tmp_path, PICKY, cases)
    return fallback_mod.Target(problem, Path("submissions/sol.cpp"), ("mylib/fast.hpp",)), testcases


def test_check_judges_every_case_through_the_emulator(tmp_path, local_env, monkeypatch):
    target, testcases = make_target(tmp_path, {"a": ("1\n", "1\n"), "b": ("2\n", "2\n")})
    monkeypatch.setattr(fallback_mod.fetch, "ensure", lambda p, **kw: testcases)
    outcome = fallback_mod.check(target, local_env, emulator=PASS_THROUGH, tle_scale=1)
    assert outcome.status == "WA"
    assert outcome.case_count == 2
    assert outcome.failed_cases == ("b",)
    assert outcome.failed_case["name"] == "b"


def test_check_reports_an_instruction_the_cpu_lacks(tmp_path, local_env, monkeypatch):
    # 本当に SIGILL で落とすと、macOS ではそのたびにクラッシュレポートが作られ、待たされる
    # ことがある。QEMU が CPU に無い命令で死んだときの結果を返す形にする。
    target, testcases = make_target(tmp_path, {"a": ("1\n", "1\n")})
    monkeypatch.setattr(fallback_mod.fetch, "ensure", lambda p, **kw: testcases)
    calls = []

    def illegal(binary, **kw):
        calls.append(kw["wrapper"])
        return result(exit_code=None, term_signal=signal.SIGILL)

    monkeypatch.setattr(fallback_mod.execute, "run", illegal)
    outcome = fallback_mod.check(target, local_env, emulator=fallback_mod.EMULATOR, tle_scale=1)
    assert calls == [fallback_mod.EMULATOR]
    assert outcome.status == "RE"
    assert "SIGILL" in outcome.failed_case["detail"]


def test_check_skips_when_testdata_is_unavailable(tmp_path, local_env, monkeypatch):
    target, _ = make_target(tmp_path, {"a": ("1\n", "1\n")})

    def unavailable(problem, **kw):
        raise fallback_mod.fetch.FetchError("保管庫に無い")

    monkeypatch.setattr(fallback_mod.fetch, "ensure", unavailable)
    outcome = fallback_mod.check(target, local_env, emulator=PASS_THROUGH, tle_scale=1)
    assert outcome.status == fallback_mod.SKIP
    assert fallback_mod.failures([outcome]) == []


def test_check_runs_an_exit_code_problem_once(tmp_path, local_env):
    problem, _ = make_layout(tmp_path)
    target = fallback_mod.Target(problem, Path("submissions/b.cpp"), ("mylib/plain.hpp",))
    # 組むときの探索パスは本物の lib/ なので、一時的な lib/ を読まない形に書き換える。
    (problem.dir / "submissions" / "b.cpp").write_text("int main() { return 0; }\n")
    outcome = fallback_mod.check(target, local_env, emulator=PASS_THROUGH, tle_scale=1)
    assert (outcome.status, outcome.case_count) == ("AC", 1)


# --- CI ------------------------------------------------------------------------


def test_the_workflow_installs_what_the_check_needs():
    """x64-gcc のコンパイラを上げて judge.yml を直し忘れると、このジョブだけが毎回落ちる。"""
    from tests.test_environment import workflow

    job = workflow()["jobs"]["fallback"]
    assert job["needs"] == "plan"
    install = next(s for s in job["steps"] if s.get("name") == "コンパイラと QEMU を入れる")
    assert env_mod.load(fallback_mod.BASE_ENV).cxx in install["run"]
    assert "qemu-user" in install["run"] and fallback_mod.EMULATOR[0] in install["run"]
    lib = next(s for s in job["steps"] if s.get("name") == "ライブラリを取る")
    assert lib["env"]["LIBRARY_SHA"] == "${{ needs.plan.outputs.library_sha }}"
    # 取れなかったら対象を選べず、何も確かめずに通ってしまう。
    assert not lib.get("continue-on-error")


# --- 書き出し ------------------------------------------------------------------


def test_report_and_summary(tmp_path, local_env):
    problem, _ = make_layout(tmp_path)
    ok = fallback_mod.Outcome("p", "submissions/a.hpp", ("mylib/x.hpp",), "AC", case_count=3)
    bad = fallback_mod.Outcome(
        "q|r", "submissions/b.hpp", ("mylib/x.hpp",), "RE", case_count=3,
        failed_cases=("c",), failed_case={"name": "c", "status": "RE", "detail": "SIGILL: x\ny"},
    )
    excluded = [fallback_mod.Excluded(problem, Path("submissions/c.cpp"), ("mylib/fast.hpp",), "提出の側にも印がある")]
    data = fallback_mod.report([ok, bad], excluded, local_env, fallback_mod.EMULATOR)
    assert data["schema"] == fallback_mod.REPORT_SCHEMA
    assert data["emulator"].startswith("qemu-x86_64 -cpu Haswell-noTSX")
    assert [r["status"] for r in data["results"]] == ["AC", "RE"]
    assert data["excluded"][0]["reason"] == "提出の側にも印がある"
    text = fallback_mod.summary_markdown([ok, bad], excluded)
    assert "対象 2 本: AC 1 本、失敗 1 本、確かめられなかった 0 本" in text
    # 表のマスの中の縦棒と改行は崩れないように逃がす。
    assert "| q\\|r |" in text and "c: SIGILL: x |" in text
    assert "除いた提出 1 本" in text
