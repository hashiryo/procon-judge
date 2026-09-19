"""環境の定義、CPU モデルの検出、CI のマトリクスとのつじつま。

環境名も cxxflags も CPU モデルもキーの材料なので、ここがずれると
同じものを測り直したり、別のものを同じ記録として扱ったりする。
"""

from __future__ import annotations

from pathlib import Path

import yaml

from pj import build as build_mod
from pj import environment as env_mod
from pj import problem as problem_mod
from pj.paths import SIMDE_DIR

ROOT = Path(__file__).resolve().parent.parent
WORKFLOW = ROOT / ".github" / "workflows" / "judge.yml"

# environments.toml の cxx から、CI がどちらのツールチェインを入れるかが決まる。
TOOLCHAIN_OF = {"g++": "gcc", "clang++": "clang"}


def ci_envs() -> list[env_mod.Environment]:
    """CI で走らせる環境。runs_on = "self" は手元専用。"""
    return [e for e in env_mod.load_all() if e.runs_on != "self"]


def matrix_entries() -> list[dict]:
    workflow = yaml.safe_load(WORKFLOW.read_text())
    return workflow["jobs"]["run"]["strategy"]["matrix"]["include"]


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
        "-Ilib -Iproblems/yosupo-point-add-range-sum -Ithird_party/simde"
    )


def test_simde_stays_in_the_include_dirs_even_if_it_is_not_checked_out():
    """submodule の有無でキーが変わってはいけない。"""
    problem = problem_mod.load_by_id("yosupo-point-add-range-sum")
    assert SIMDE_DIR in build_mod.include_dirs(problem)


# --- CI のマトリクス -------------------------------------------------------


def test_matrix_covers_every_ci_environment():
    """environments.toml に足して judge.yml を忘れると、その環境は黙って走らない。"""
    assert {e.name for e in ci_envs()} == {m["env"] for m in matrix_entries()}


def test_matrix_runner_matches_the_definition():
    runs_on = {e.name: e.runs_on for e in ci_envs()}
    for entry in matrix_entries():
        assert entry["runs_on"] == runs_on[entry["env"]]


def test_matrix_toolchain_matches_the_compiler():
    toolchain = {e.name: TOOLCHAIN_OF[e.cxx.rsplit("-", 1)[0]] for e in ci_envs()}
    for entry in matrix_entries():
        assert entry["toolchain"] == toolchain[entry["env"]]


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
