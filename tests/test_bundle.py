"""pj bundle。提出を判定サイトへ貼れる 1 ファイルに展開する。"""

import io
from pathlib import Path

import pytest

from pj import bundle as bundle_mod
from pj import cli
from pj import environment as env_mod
from pj import problem as problem_mod

EXIT_CODE_TOML = """
id = "{id}"
title = "t"

[harness]
kind = "{kind}"

[testdata]
source = "none"

[compare]
kind = "exit_code"
"""

BASE_CPP = """\
#include "pj.hpp"

// CI では -DSUBMISSION_HPP で上書きされる。
#ifndef SUBMISSION_HPP
#define SUBMISSION_HPP "submissions/naive.hpp"
#endif
#include SUBMISSION_HPP

int main() { return run() == 42 ? 0 : 1; }
"""


def write(directory, name, text):
    path = directory / name
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(text)
    return path


@pytest.fixture
def isa(tmp_path, monkeypatch):
    """先頭に置く宣言。lib/ が無くても (CI の test ジョブ) 回るように一時ファイルに替える。"""
    path = write(tmp_path / "isa", "isa-pragma.hpp", "// isa\n")
    monkeypatch.setattr(bundle_mod, "ISA_PRAGMA", path)
    monkeypatch.setattr(bundle_mod, "BUNDLE_CACHE_DIR", tmp_path / "bundle-cache")
    return path


def make_problem(tmp_path, kind, files):
    directory = tmp_path / f"tmp-{kind}"
    write(directory, "problem.toml", EXIT_CODE_TOML.format(id=directory.name, kind=kind))
    for name, text in files.items():
        write(directory, name, text)
    return problem_mod.load(directory)


def lines(text):
    return [line for line in text.splitlines() if line.strip()]


# --- 入口の置き換え ------------------------------------------------------------


def test_the_submission_block_becomes_one_include():
    text = bundle_mod.select_submission(BASE_CPP, Path("submissions/lib.hpp"))
    assert '#include "submissions/lib.hpp"' in text
    assert "#ifndef SUBMISSION_HPP" not in text
    assert "#define SUBMISSION_HPP" not in text
    # 前後はそのまま。コメントの中の SUBMISSION_HPP は残ってよい。
    assert text.startswith('#include "pj.hpp"\n')
    assert "// CI では -DSUBMISSION_HPP で上書きされる。" in text
    assert text.rstrip().endswith("int main() { return run() == 42 ? 0 : 1; }")


def test_a_bare_include_of_the_macro_is_also_replaced():
    text = bundle_mod.select_submission(
        "#include SUBMISSION_HPP\nint main() {}\n", Path("submissions/a.hpp")
    )
    assert text == '#include "submissions/a.hpp"\nint main() {}\n'


def test_a_base_without_the_include_is_refused():
    with pytest.raises(bundle_mod.BundleError):
        bundle_mod.select_submission("int main() {}\n", Path("submissions/a.hpp"))


def test_other_uses_of_the_macro_are_refused():
    # 測ったものと中身が変わるので、黙って展開しない。
    text = '#include SUBMISSION_HPP\nconst char *name = SUBMISSION_HPP;\n'
    with pytest.raises(bundle_mod.BundleError):
        bundle_mod.select_submission(text, Path("submissions/a.hpp"))


# --- 展開 ----------------------------------------------------------------------


def test_quoted_includes_are_inlined_in_place(tmp_path):
    write(tmp_path, "a.hpp", "int a;\n")
    bundled = bundle_mod.expand(
        'int before;\n#include "a.hpp"\nint after;\n', tmp_path, [tmp_path]
    )
    assert lines(bundled.text) == ["int before;", "int a;", "int after;"]
    assert bundled.files == ((tmp_path / "a.hpp").resolve(),)


def test_angle_brackets_are_kept(tmp_path):
    write(tmp_path, "vector", "should not be read\n")
    bundled = bundle_mod.expand("#include <vector>\n", tmp_path, [tmp_path])
    assert lines(bundled.text) == ["#include <vector>"]
    assert bundled.files == ()


def test_a_file_is_inlined_once(tmp_path):
    # 菱形。d は b と c の両方から読まれるが、中身は 1 回だけ入る。
    write(tmp_path, "d.hpp", "#pragma once\nint d;\n")
    write(tmp_path, "b.hpp", '#pragma once\n#include "d.hpp"\nint b;\n')
    write(tmp_path, "c.hpp", '#pragma once\n#include "d.hpp"\nint c;\n')
    bundled = bundle_mod.expand(
        '#include "b.hpp"\n#include "c.hpp"\n#include "b.hpp"\n', tmp_path, [tmp_path]
    )
    assert lines(bundled.text) == ["int d;", "int b;", "int c;"]


def test_pragma_once_is_dropped(tmp_path):
    write(tmp_path, "a.hpp", "#pragma once\n  #  pragma   once\nint a;\n")
    bundled = bundle_mod.expand('#include "a.hpp"\n', tmp_path, [tmp_path])
    assert "pragma" not in bundled.text


def test_the_including_directory_is_searched_first(tmp_path):
    lib = tmp_path / "lib"
    write(lib, "mylib/x.hpp", '#include "y.hpp"\n')
    write(lib, "mylib/y.hpp", "int from_mylib;\n")
    write(lib, "y.hpp", "int from_lib_root;\n")
    bundled = bundle_mod.expand('#include "mylib/x.hpp"\n', tmp_path, [lib])
    assert lines(bundled.text) == ["int from_mylib;"]


def test_search_paths_are_tried_in_order(tmp_path):
    first, second = tmp_path / "first", tmp_path / "second"
    write(first, "x.hpp", "int from_first;\n")
    write(second, "x.hpp", "int from_second;\n")
    write(second, "only.hpp", "int only_in_second;\n")
    bundled = bundle_mod.expand(
        '#include "x.hpp"\n#include "only.hpp"\n', tmp_path / "entry", [first, second]
    )
    assert lines(bundled.text) == ["int from_first;", "int only_in_second;"]


def test_unresolved_includes_are_kept_and_reported(tmp_path):
    bundled = bundle_mod.expand(
        '#include "missing.hpp"\n#include "missing.hpp"\n', tmp_path, [tmp_path]
    )
    assert lines(bundled.text) == ['#include "missing.hpp"', '#include "missing.hpp"']
    assert bundled.unresolved == ("missing.hpp",)


def test_the_prelude_comes_first_and_only_once(tmp_path):
    prelude = write(tmp_path, "isa.hpp", "// isa\n")
    bundled = bundle_mod.expand(
        '#include "isa.hpp"\nint main() {}\n', tmp_path, [tmp_path], prelude=[prelude]
    )
    assert lines(bundled.text) == ["// isa", "int main() {}"]


def test_the_entry_is_not_inlined_into_itself(tmp_path):
    entry = write(tmp_path, "a.cpp", '#include "b.hpp"\nint main() {}\n')
    write(tmp_path, "b.hpp", '#include "a.cpp"\nint b;\n')
    bundled = bundle_mod.expand(entry.read_text(), tmp_path, [tmp_path], entry=entry)
    assert lines(bundled.text) == ["int b;", "int main() {}"]


# --- 問題から展開する -----------------------------------------------------------


def test_a_base_problem_is_the_harness_with_the_submission(tmp_path, isa):
    problem = make_problem(tmp_path, "base", {
        "base.cpp": BASE_CPP,
        "submissions/lib.hpp": '#include "common.hpp"\ninline int run() { return answer(); }\n',
        "submissions/naive.hpp": "inline int run() { return 0; }\n",
        "common.hpp": "#pragma once\ninline int answer() { return 42; }\n",
    })
    bundled = bundle_mod.bundle(problem, Path("submissions/lib.hpp"))
    body = bundled.text
    assert body.startswith("// isa\n")
    # ハーネスの pj.hpp も中身が入る。
    assert "#include \"pj.hpp\"" not in body and "must_scan" in body
    assert "inline int answer() { return 42; }" in body
    assert "SUBMISSION_HPP" not in body.replace("-DSUBMISSION_HPP", "")
    # 既定の提出 (naive) は入らない。
    assert "inline int run() { return 0; }" not in body
    assert bundled.files[0] == isa.resolve()


def test_a_raw_problem_is_the_submission_itself(tmp_path, isa):
    problem = make_problem(tmp_path, "raw", {
        "submissions/a.cpp": '#include "common.hpp"\nint main() { return answer() - 42; }\n',
        "common.hpp": "inline int answer() { return 42; }\n",
    })
    bundled = bundle_mod.bundle(problem, Path("submissions/a.cpp"))
    assert lines(bundled.text) == [
        "// isa", "inline int answer() { return 42; }", "int main() { return answer() - 42; }",
    ]


def test_a_missing_isa_pragma_is_an_error(tmp_path, isa, monkeypatch):
    monkeypatch.setattr(bundle_mod, "ISA_PRAGMA", tmp_path / "nowhere.hpp")
    problem = make_problem(tmp_path, "raw", {"submissions/a.cpp": "int main() {}\n"})
    with pytest.raises(bundle_mod.BundleError, match="nowhere.hpp"):
        bundle_mod.bundle(problem, Path("submissions/a.cpp"))


def test_the_source_limit_comes_from_the_id_prefix(tmp_path):
    def limit(pid):
        directory = tmp_path / pid
        write(directory, "problem.toml", EXIT_CODE_TOML.format(id=pid, kind="raw"))
        return bundle_mod.source_limit(problem_mod.load(directory))

    assert limit("cf-1310-f") == ("Codeforces", 65536)
    assert limit("atcoder-abc172-d") == ("AtCoder", 524288)
    assert limit("self-gf2-64-pow") is None


# --- --check -------------------------------------------------------------------


def test_check_flags_drop_includes_and_arch():
    x64 = env_mod.load("x64-gcc")
    assert bundle_mod.check_flags(x64) == ["-std=gnu++23", "-O2", "-fconstexpr-ops-limit=2097152"]


def test_check_flags_carry_the_constexpr_limit_of_the_family():
    assert "-fconstexpr-steps=524288" in bundle_mod.check_flags(env_mod.load("x64-clang"))


@pytest.mark.parametrize(
    ("version", "flag"),
    [
        ("Apple clang version 21.0.0 (clang-2100.1.1.101)", "-fconstexpr-steps=524288"),
        ("c++ (Ubuntu 13.3.0-6ubuntu2~24.04) 13.3.0", "-fconstexpr-ops-limit=2097152"),
    ],
)
def test_check_flags_read_the_family_of_local_off_the_version(monkeypatch, version, flag):
    # local は environments.toml に上限を書けない (c++ が macOS と ubuntu で別のコンパイラ)。
    monkeypatch.setattr(env_mod, "compiler_version", lambda cxx: version)
    flags = bundle_mod.check_flags(env_mod.load("local"))
    assert flag in flags
    assert len([f for f in flags if f.startswith("-fconstexpr-")]) == 1


def test_check_flags_keep_the_simde_switch():
    # arm と macOS は x86 の intrinsics を SIMDe に読み替えるので、-D と SIMDe の -I を残す。
    flags = bundle_mod.check_flags(env_mod.load("arm-gcc"))
    assert "-DUSE_SIMDE" in flags and "-DSIMDE_ENABLE_NATIVE_ALIASES" in flags
    assert any(f.startswith("-I") and f.endswith("simde") for f in flags)
    assert not any(f.startswith(("-mcpu", "-march", "-flto")) for f in flags)


def bundle_and_check(problem, submission):
    bundled = bundle_mod.bundle(problem, Path(submission))
    path = bundle_mod.output_path(problem, Path(submission))
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(bundled.text)
    out = io.StringIO()
    code = bundle_mod.check(problem, Path(submission), path, env_mod.load("local"), out=out)
    return code, out.getvalue()


def test_check_builds_the_bundle_alone_and_runs_it(tmp_path, isa):
    problem = make_problem(tmp_path, "base", {
        "base.cpp": BASE_CPP,
        "submissions/lib.hpp": '#include "common.hpp"\ninline int run() { return answer(); }\n',
        "common.hpp": "inline int answer() { return 42; }\n",
    })
    code, out = bundle_and_check(problem, "submissions/lib.hpp")
    assert code == 0, out
    assert "AC" in out


def test_check_catches_a_header_left_behind(tmp_path, isa):
    # 山括弧で読んだ手元のヘッダは展開に入らない。-I を付けずに組むので、ここで分かる。
    problem = make_problem(tmp_path, "raw", {
        "submissions/a.cpp": "#include <common.hpp>\nint main() { return answer() - 42; }\n",
        "common.hpp": "inline int answer() { return 42; }\n",
    })
    code, out = bundle_and_check(problem, "submissions/a.cpp")
    assert code == 1
    assert "CE" in out and "common.hpp" in out


def test_check_reports_a_failing_run(tmp_path, isa):
    problem = make_problem(tmp_path, "raw", {"submissions/a.cpp": "int main() { return 3; }\n"})
    code, out = bundle_and_check(problem, "submissions/a.cpp")
    assert code == 1
    assert "RE" in out


# --- CLI -----------------------------------------------------------------------


@pytest.fixture
def raw_problem(tmp_path, isa, monkeypatch):
    problem = make_problem(tmp_path, "raw", {
        "submissions/a.cpp": '#include "common.hpp"\nint main() { return answer() - 42; }\n',
        "common.hpp": "inline int answer() { return 42; }\n",
    })
    monkeypatch.setattr(cli.problem_mod, "load_by_id", lambda pid: problem)
    return problem


def test_cli_writes_the_file_and_prints_the_source(raw_problem, capsys):
    assert cli.main(["bundle", "--problem", raw_problem.id, "--submission", "a.cpp"]) == 0
    captured = capsys.readouterr()
    path = bundle_mod.output_path(raw_problem, Path("submissions/a.cpp"))
    assert path.read_text() == captured.out
    assert "inline int answer()" in captured.out
    # 知らせは stderr に出す。標準出力はそのまま貼れる。
    assert str(path) in captured.err


def test_cli_out_changes_the_destination(raw_problem, tmp_path, capsys):
    dest = tmp_path / "somewhere" / "x.cpp"
    assert cli.main([
        "bundle", "--problem", raw_problem.id, "--submission", "a.cpp", "--out", str(dest),
    ]) == 0
    assert dest.read_text() == capsys.readouterr().out


def test_cli_copy_goes_to_the_clipboard_instead_of_stdout(raw_problem, monkeypatch, capsys):
    copied = []
    monkeypatch.setattr(bundle_mod, "copy_to_clipboard", copied.append)
    assert cli.main([
        "bundle", "--problem", raw_problem.id, "--submission", "a.cpp", "--copy",
    ]) == 0
    captured = capsys.readouterr()
    assert captured.out == ""
    assert len(copied) == 1 and "inline int answer()" in copied[0]


def test_cli_warns_over_the_source_limit(raw_problem, monkeypatch, capsys):
    monkeypatch.setattr(bundle_mod, "source_limit", lambda p: ("Codeforces", 10))
    assert cli.main(["bundle", "--problem", raw_problem.id, "--submission", "a.cpp"]) == 0
    assert "Codeforces のソースの上限 10 bytes を超えています" in capsys.readouterr().err


def test_the_limit_counts_newlines_as_crlf(raw_problem, monkeypatch, capsys):
    # 貼ったソースはブラウザが CRLF で送る。LF のままでは収まっても、CRLF で超えるなら警告する。
    bundled = bundle_mod.bundle(raw_problem, Path("submissions/a.cpp"))
    assert bundled.crlf_size == bundled.size + bundled.text.count("\n")
    monkeypatch.setattr(bundle_mod, "source_limit", lambda p: ("AtCoder", bundled.size))
    assert cli.main(["bundle", "--problem", raw_problem.id, "--submission", "a.cpp"]) == 0
    err = capsys.readouterr().err
    assert f"改行を CRLF で数えると {bundled.crlf_size} bytes で、AtCoder のソースの上限" in err


def test_a_bundle_within_the_limit_does_not_warn(raw_problem, monkeypatch, capsys):
    bundled = bundle_mod.bundle(raw_problem, Path("submissions/a.cpp"))
    monkeypatch.setattr(bundle_mod, "source_limit", lambda p: ("AtCoder", bundled.crlf_size))
    assert cli.main(["bundle", "--problem", raw_problem.id, "--submission", "a.cpp"]) == 0
    assert "warning" not in capsys.readouterr().err


def test_cli_cases_needs_check(raw_problem, capsys):
    assert cli.main([
        "bundle", "--problem", raw_problem.id, "--submission", "a.cpp", "--cases", "2",
    ]) == 1
    assert "--check" in capsys.readouterr().err


def test_cli_check_returns_the_verdict(raw_problem, capsys):
    assert cli.main([
        "bundle", "--problem", raw_problem.id, "--submission", "a.cpp", "--check",
    ]) == 0
    assert "終了コード 0 で走り切りました" in capsys.readouterr().err
