"""参考に落ちた理由。どのファイルが動いたかを名指しできるか。"""

from __future__ import annotations

from dataclasses import replace
from pathlib import Path

import pytest

from pj import environment as env_mod
from pj import problem as problem_mod
from pj import run as run_mod
from pj.freshness import Diff, Freshness

RAW_TOML = """
id = "tmp-diff"
title = "t"

[limits]
tle_sec = 5.0
mle_mb = 256

[harness]
kind = "raw"

[testdata]
source = "none"

[compare]
kind = "compile_only"
"""

BASE_TOML = RAW_TOML.replace('kind = "raw"', 'kind = "base"')

SOURCE = '#include "dep.hpp"\nint main() { return g(); }\n'
DEP = "int g() { return 0; }\n"

MACHINE = run_mod.Machine(cpu_arch="x86_64", cpu_model="EPYC", compiler_version="g++-15")


@pytest.fixture
def envs():
    return env_mod.load_all()


def make_problem(tmp_path, *, toml=RAW_TOML, base_cpp=None):
    directory = tmp_path / "tmp-diff"
    directory.mkdir()
    (directory / "problem.toml").write_text(toml)
    (directory / "dep.hpp").write_text(DEP)
    (directory / "submissions").mkdir()
    (directory / "submissions" / "a.cpp").write_text(SOURCE)
    if base_cpp is not None:
        (directory / "base.cpp").write_text(base_cpp)
    return problem_mod.load(directory)


def record_for(problem, env):
    """今のソースをこの環境で測ったことにした記録。run と同じ道具で作る。"""
    decided = run_mod._decide(
        problem, [Path("submissions/a.cpp")], "h", env, MACHINE, set()
    )
    assert decided.jobs, decided
    return {**run_mod.describe(decided.jobs[0]), "timestamp": "2026-01-01T00:00:00Z"}


def rewrite(problem, name, text):
    (problem.dir / name).write_text(text)


def test_a_current_record_has_no_diff(tmp_path, envs):
    problem = make_problem(tmp_path)
    record = record_for(problem, envs[0])
    assert Freshness(problem, envs).diff(record) is None


def test_the_record_carries_the_key_components(tmp_path, envs):
    problem = make_problem(tmp_path)
    record = record_for(problem, envs[0])
    assert record["harness_hash"]
    assert record["problem_hash"]
    assert set(record["file_hashes"]) == {"submissions/a.cpp", "dep.hpp"}


def test_editing_a_header_names_the_file(tmp_path, envs):
    problem = make_problem(tmp_path)
    record = record_for(problem, envs[0])
    rewrite(problem, "dep.hpp", "int g() { return 1; }\n")
    assert Freshness(problem, envs).diff(record) == Diff(changed=("dep.hpp",))


def test_editing_the_submission_names_it(tmp_path, envs):
    problem = make_problem(tmp_path)
    record = record_for(problem, envs[0])
    rewrite(problem, "submissions/a.cpp", SOURCE.replace("g()", "g() + 1"))
    assert Freshness(problem, envs).diff(record) == Diff(changed=("submissions/a.cpp",))


def test_a_new_include_is_added(tmp_path, envs):
    problem = make_problem(tmp_path)
    record = record_for(problem, envs[0])
    rewrite(problem, "extra.hpp", "int h();\n")
    rewrite(problem, "submissions/a.cpp", '#include "extra.hpp"\n' + SOURCE)
    assert Freshness(problem, envs).diff(record) == Diff(
        changed=("submissions/a.cpp",), added=("extra.hpp",)
    )


def test_a_dropped_include_is_removed(tmp_path, envs):
    problem = make_problem(tmp_path)
    record = record_for(problem, envs[0])
    rewrite(problem, "submissions/a.cpp", "int main() { return 0; }\n")
    assert Freshness(problem, envs).diff(record) == Diff(
        changed=("submissions/a.cpp",), removed=("dep.hpp",)
    )


def test_a_reformatted_header_is_not_a_reason(tmp_path, envs):
    problem = make_problem(tmp_path)
    record = record_for(problem, envs[0])
    rewrite(problem, "dep.hpp", "// g\nint g() {\n  return 0;\n}\n")
    assert Freshness(problem, envs).diff(record) is None


def test_changed_flags_are_a_setting(tmp_path, envs):
    """environments.toml のフラグが変わると、記録の cxxflags と食い違う。"""
    problem = make_problem(tmp_path)
    record = record_for(problem, envs[0])
    changed = [replace(envs[0], cxxflags=envs[0].cxxflags + " -DEXTRA"), *envs[1:]]
    assert Freshness(problem, changed).diff(record) == Diff(settings=("cxxflags",))


def test_changed_limits_are_a_setting(tmp_path, envs):
    problem = make_problem(tmp_path)
    record = record_for(problem, envs[0])
    rewrite(problem, "problem.toml", RAW_TOML.replace("tle_sec = 5.0", "tle_sec = 4.0"))
    reloaded = problem_mod.load(problem.dir)
    assert Freshness(reloaded, envs).diff(record) == Diff(settings=("problem",))


def test_an_old_record_has_no_reason(tmp_path, envs):
    """file_hashes を持たない記録は、参考であることしか言えない。"""
    problem = make_problem(tmp_path)
    record = record_for(problem, envs[0])
    for name in ("file_hashes", "harness_hash", "problem_hash"):
        del record[name]
    rewrite(problem, "dep.hpp", "int g() { return 1; }\n")
    diff = Freshness(problem, envs).diff(record)
    assert diff == Diff()
    assert diff.known is False


def test_an_undeterminable_record_has_no_diff(tmp_path, envs):
    problem = make_problem(tmp_path)
    record = record_for(problem, envs[0])
    (problem.dir / "submissions" / "a.cpp").unlink()
    assert Freshness(problem, envs).diff(record) is None


def test_harness_files_are_named_too(tmp_path, envs):
    problem = make_problem(tmp_path, toml=BASE_TOML, base_cpp="int main() {}\n")
    record = record_for(problem, envs[0])
    assert "base.cpp" in record["file_hashes"]
    rewrite(problem, "base.cpp", "int main() { return 0; }\n")
    assert Freshness(problem, envs).diff(record) == Diff(changed=("base.cpp",))
