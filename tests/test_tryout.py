"""pj try。手元で組んで、自分で用意した入力で走らせる。判定も記録もしない。"""

import io
from pathlib import Path

import pytest

from pj import cli, tryout
from pj import environment as env_mod
from pj import problem as problem_mod
from tests.test_run import BASE_TOML, PICKY, make_case_problem


@pytest.fixture
def local_env():
    return env_mod.load("local")


@pytest.fixture(autouse=True)
def cache_in_tmp(tmp_path, monkeypatch):
    monkeypatch.setattr(tryout, "TRY_CACHE_DIR", tmp_path / "try-cache")


def run_file(source, env, input_path=None):
    err = io.StringIO()
    code = tryout.run_built(tryout.build_file(source, env), input_path=input_path, err=err)
    return code, err.getvalue()


def test_a_file_runs_with_the_given_input(tmp_path, local_env, capfd):
    source = tmp_path / "a.cpp"
    source.write_text(
        "#include <cstdio>\n"
        "int main() {\n"
        "  int n;\n"
        "  if (scanf(\"%d\", &n) != 1) return 2;\n"
        "  printf(\"%d\\n\", n * 2);\n"
        "  fprintf(stderr, \"debug n=%d\\n\", n);\n"
        "}\n"
    )
    stdin = tmp_path / "in.txt"
    stdin.write_text("21\n")
    code, err = run_file(source, local_env, stdin)
    out = capfd.readouterr()
    assert code == 0
    assert "42" in out.out
    # stderr は捕まえずに端末へ流す。
    assert "debug n=21" in out.err
    assert "終了コード 0" in err


def test_local_is_defined_for_the_debug_macros(tmp_path, local_env, capfd):
    source = tmp_path / "a.cpp"
    source.write_text(
        "#include <cstdio>\n"
        "int main() {\n"
        "#ifdef __LOCAL\n"
        "  puts(\"local\");\n"
        "#endif\n"
        "}\n"
    )
    code, _ = run_file(source, local_env)
    assert code == 0
    assert "local" in capfd.readouterr().out


def test_a_file_sees_the_harness_directory(tmp_path, local_env):
    # 提出と同じ探索パスで組む。harness/ の pj.hpp が引ける。
    source = tmp_path / "a.cpp"
    source.write_text('#include "pj.hpp"\nint main() { return 0; }\n')
    code, err = run_file(source, local_env)
    assert code == 0, err


def test_the_exit_code_is_passed_through(tmp_path, local_env):
    source = tmp_path / "a.cpp"
    source.write_text("int main() { return 3; }\n")
    code, err = run_file(source, local_env)
    assert code == 3
    assert "終了コード 3" in err


def test_a_compile_error_is_reported(tmp_path, local_env):
    source = tmp_path / "a.cpp"
    source.write_text("int main() { return nope; }\n")
    code, err = run_file(source, local_env)
    assert code == 1
    assert "CE" in err and "nope" in err


def test_a_base_submission_runs_through_its_harness(tmp_path, local_env, capfd):
    directory = tmp_path / "tmp-base"
    directory.mkdir()
    (directory / "problem.toml").write_text(BASE_TOML)
    (directory / "base.cpp").write_text(
        "#include <cstdio>\n"
        "#include SUBMISSION_HPP\n"
        "int main() { int n; if (scanf(\"%d\", &n) != 1) return 2; printf(\"%d\\n\", twice(n)); }\n"
    )
    (directory / "submissions").mkdir()
    (directory / "submissions" / "a.hpp").write_text("inline int twice(int n) { return 2 * n; }\n")
    problem = problem_mod.load(directory)
    stdin = tmp_path / "in.txt"
    stdin.write_text("5\n")

    err = io.StringIO()
    built = tryout.build_submission(problem, Path("submissions/a.hpp"), local_env)
    code = tryout.run_built(built, input_path=stdin, err=err)
    assert code == 0, err.getvalue()
    assert "10" in capfd.readouterr().out


def test_a_case_of_the_problem_is_passed_as_input(tmp_path, local_env, monkeypatch, capfd):
    problem, testcases = make_case_problem(tmp_path, PICKY, {"a": ("1\n", "1\n"), "b": ("2\n", "2\n")})
    monkeypatch.setattr(tryout.fetch, "ensure", lambda p, **kw: testcases)
    case = tryout.find_case(problem, local_env, "a")
    err = io.StringIO()
    built = tryout.build_submission(problem, Path("submissions/sol.cpp"), local_env)
    code = tryout.run_built(built, input_path=case.in_path, expected_path=case.out_path, err=err)
    assert code == 0, err.getvalue()
    assert capfd.readouterr().out.strip() == "1"
    # 判定はせず、見比べられるよう期待出力の場所だけを出す。
    assert f"期待出力   {case.out_path}" in err.getvalue()


def test_an_unknown_case_lists_some_names(tmp_path, local_env, monkeypatch):
    problem, testcases = make_case_problem(tmp_path, PICKY, {"a": ("1\n", "1\n"), "b": ("2\n", "2\n")})
    monkeypatch.setattr(tryout.fetch, "ensure", lambda p, **kw: testcases)
    with pytest.raises(tryout.TryError, match="例: a, b"):
        tryout.find_case(problem, local_env, "zzz")


def test_a_problem_without_testdata_has_no_cases(tmp_path, local_env):
    directory = tmp_path / "tmp-base"
    directory.mkdir()
    (directory / "problem.toml").write_text(BASE_TOML)
    (directory / "base.cpp").write_text("#include SUBMISSION_HPP\nint main() { return 0; }\n")
    problem = problem_mod.load(directory)
    with pytest.raises(tryout.TryError, match="テストデータを使わない"):
        tryout.find_case(problem, local_env, "a")


def test_the_cli_checks_how_the_arguments_are_combined(capsys):
    assert cli.main(["try"]) == 1
    assert "ファイルか、--problem と --submission の組" in capsys.readouterr().err
    assert cli.main(["try", "a.cpp", "--problem", "p", "--submission", "submissions/a.hpp"]) == 1
    assert "一緒に渡せません" in capsys.readouterr().err
    assert cli.main(["try", "--problem", "p"]) == 1
    assert "両方" in capsys.readouterr().err
    assert cli.main(["try", "a.cpp", "--case", "x"]) == 1
    assert "--case は --problem と一緒に" in capsys.readouterr().err
    assert cli.main(["try", "a.cpp", "--problem", "p"]) == 1
    assert "--case でその問題のケース" in capsys.readouterr().err


def test_the_cli_rejects_a_missing_file(tmp_path, capsys):
    assert cli.main(["try", str(tmp_path / "missing.cpp")]) == 1
    assert "がありません" in capsys.readouterr().err
