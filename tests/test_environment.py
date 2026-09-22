"""環境の定義、CPU モデルの検出、CI のマトリクスとのつじつま。

環境名も cxxflags も CPU モデルもキーの材料なので、ここがずれると
同じものを測り直したり、別のものを同じ記録として扱ったりする。
"""

from __future__ import annotations

from pathlib import Path

import pytest
import yaml

from pj import build as build_mod
from pj import environment as env_mod
from pj import problem as problem_mod
from pj.paths import HARNESS_DIR, SIMDE_DIR

ROOT = Path(__file__).resolve().parent.parent
WORKFLOW = ROOT / ".github" / "workflows" / "judge.yml"

def ci_envs() -> list[env_mod.Environment]:
    """CI で走らせる環境。runs_on = "self" は手元専用。"""
    return [e for e in env_mod.load_all() if e.runs_on != "self"]


def workflow() -> dict:
    return yaml.safe_load(WORKFLOW.read_text())


# --- environments.toml -----------------------------------------------------


def test_environment_names_are_unique():
    names = [e.name for e in env_mod.load_all()]
    assert len(names) == len(set(names))


def test_local_is_the_only_self_hosted_one():
    assert [e.name for e in env_mod.load_all() if e.runs_on == "self"] == ["local"]


def test_cxxflags_do_not_carry_include_dirs():
    """-I は pj が足す。両方に書くと記録の cxxflags に同じものが二度出る。"""
    for env in env_mod.load_all():
        assert "-I" not in env.cxxflags, env.name


def test_effective_cxxflags_append_the_include_dirs():
    env = env_mod.load("local")
    problem = problem_mod.load_by_id("yosupo-point-add-range-sum")
    flags = build_mod.effective_cxxflags(env, problem)
    assert flags.startswith(env.cxxflags)
    assert flags.endswith(
        "-Ilib -Iproblems/yosupo-point-add-range-sum -Iharness -Ithird_party/simde"
    )


def test_the_problem_directory_wins_over_the_shared_harness():
    """問題ごとに同じ名前のヘッダを置いたら、そちらを先に見る。"""
    problem = problem_mod.load_by_id("yosupo-point-add-range-sum")
    dirs = build_mod.include_dirs(problem)
    assert dirs.index(problem.dir) < dirs.index(HARNESS_DIR)


def test_simde_stays_in_the_include_dirs_even_if_it_is_not_checked_out():
    """submodule の有無でキーが変わってはいけない。"""
    problem = problem_mod.load_by_id("yosupo-point-add-range-sum")
    assert SIMDE_DIR in build_mod.include_dirs(problem)


# --- CI のマトリクス -------------------------------------------------------


def test_the_run_job_is_named_after_the_group_and_the_number():
    """# から後ろが YAML のコメントになって名前が切れていたことがある。"""
    name = workflow()["jobs"]["run"]["name"]
    assert "${{ matrix.group }}" in name
    assert "${{ matrix.job }}" in name


def test_groups_follow_the_machine():
    """x64-gcc と x64-clang は同じマシンに載るので 1 本のジョブが両方を測る。"""
    groups = env_mod.groups(env_mod.load_all())
    assert set(groups) == {"x64", "arm"}
    assert [e.name for e in groups["x64"]] == ["x64-gcc", "x64-clang"]
    assert "local" not in {e.name for members in groups.values() for e in members}


def test_a_group_must_share_its_machine():
    envs = [
        env_mod.Environment("x64-gcc", "ubuntu-24.04", "g++-15", ""),
        env_mod.Environment("x64-clang", "ubuntu-22.04", "clang++-21", ""),
    ]
    with pytest.raises(env_mod.EnvironmentError_):
        env_mod.groups(envs)


def test_the_matrix_comes_from_the_plan_job():
    """手書きの matrix に戻すと、やることが 0 件でもジョブが立つ。"""
    run = workflow()["jobs"]["run"]
    assert run["needs"] == "plan"
    assert run["strategy"]["matrix"] == "${{ fromJSON(needs.plan.outputs.matrix) }}"
    assert run["if"] == "needs.plan.outputs.any == 'true'"


def test_every_compiler_is_installed_by_the_workflow():
    """cxx を上げて judge.yml を忘れると、その環境は毎回落ちる。"""
    step = next(
        s
        for s in workflow()["jobs"]["run"]["steps"]
        if s.get("name") == "コンパイラを入れる"
    )
    for env in ci_envs():
        assert env.cxx in step["run"], env.name


def test_the_matrix_toolchains_drive_the_install_loop():
    """plan が出すツールチェインの並びで、入れる側が組の全コンパイラを回す。"""
    step = next(
        s
        for s in workflow()["jobs"]["run"]["steps"]
        if s.get("name") == "コンパイラを入れる"
    )
    assert step["env"]["TOOLCHAINS"] == "${{ matrix.toolchains }}"
    assert {env_mod.toolchain(e) for e in ci_envs()} == {"gcc", "clang"}
    # 片方が入らなくても止めない。pj run が飛ばす。
    assert "set -e" not in step["run"]


def test_the_toolchain_is_read_off_the_compiler():
    assert env_mod.toolchain(env_mod.load("x64-gcc")) == "gcc"
    assert env_mod.toolchain(env_mod.load("arm-clang")) == "clang"


# --- CPU モデルの検出 ------------------------------------------------------

CPUINFO_X86 = """\
processor\t: 0
vendor_id\t: AuthenticAMD
cpu family\t: 25
model\t\t: 1
model name\t: AMD EPYC 7763 64-Core Processor
stepping\t: 1
"""

# arm の cpuinfo には名前が無く、実装者と part 番号しか載らない。
CPUINFO_AARCH64 = """\
processor\t: 0
BogoMIPS\t: 50.00
Features\t: fp asimd evtstrm aes pmull sha1 sha2 crc32
CPU implementer\t: 0x41
CPU architecture: 8
CPU variant\t: 0x0
CPU part\t: 0xd49
CPU revision\t: 0
"""

LSCPU_AARCH64 = """\
Architecture:             aarch64
  CPU op-mode(s):         64-bit
  Byte Order:             Little Endian
CPU(s):                   4
Vendor ID:                ARM
  Model name:             Neoverse-N2
    Model:                0
    Thread(s) per core:   1
"""


def test_cpuinfo_gives_the_model_name_on_x86():
    assert (
        env_mod.parse_cpuinfo_model(CPUINFO_X86) == "AMD EPYC 7763 64-Core Processor"
    )


def test_cpuinfo_gives_nothing_on_arm():
    assert env_mod.parse_cpuinfo_model(CPUINFO_AARCH64) is None


def test_lscpu_names_the_arm_core():
    assert env_mod.parse_lscpu_model(LSCPU_AARCH64) == "Neoverse-N2"


def test_lscpu_ignores_the_other_model_line():
    """lscpu には "Model:" もある。番号の方を拾ってはいけない。"""
    assert env_mod.parse_lscpu_model(LSCPU_AARCH64) != "0"
