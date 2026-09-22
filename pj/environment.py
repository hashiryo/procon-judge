"""environments.toml の読み込みと、走っているマシンの素性。"""

from __future__ import annotations

import os
import platform
import subprocess
import tomllib
from dataclasses import dataclass
from pathlib import Path

from .paths import ENVIRONMENTS_TOML

TOOLCHAINS = {"g++": "gcc", "clang++": "clang", "c++": "system"}


class EnvironmentError_(Exception):
    """環境の定義か検出で失敗したときに投げる。"""


@dataclass(frozen=True)
class Environment:
    name: str
    runs_on: str
    cxx: str
    cxxflags: str


def load_all(path: Path = ENVIRONMENTS_TOML) -> list[Environment]:
    with path.open("rb") as f:
        raw = tomllib.load(f)
    envs = []
    for entry in raw.get("env", []):
        for key in ("name", "runs_on", "cxx", "cxxflags"):
            if key not in entry:
                raise EnvironmentError_(f"{path}: env に {key} がありません: {entry}")
        envs.append(
            Environment(
                name=entry["name"],
                runs_on=entry["runs_on"],
                cxx=entry["cxx"],
                cxxflags=entry["cxxflags"],
            )
        )
    if not envs:
        raise EnvironmentError_(f"{path}: env が 1 つもありません")
    return envs


def load(name: str, path: Path = ENVIRONMENTS_TOML) -> Environment:
    for env in load_all(path):
        if env.name == name:
            return env
    known = ", ".join(e.name for e in load_all(path))
    raise EnvironmentError_(f"環境 {name!r} がありません (定義済み: {known})")


def compiler_version(cxx: str) -> str:
    try:
        proc = subprocess.run(
            [cxx, "--version"], capture_output=True, text=True, timeout=30, check=True
        )
    except (subprocess.SubprocessError, OSError) as e:
        raise EnvironmentError_(f"{cxx} --version が動きません: {e}") from e
    return proc.stdout.splitlines()[0].strip()


def _capture(cmd: list[str]) -> str | None:
    """外部コマンドの標準出力。失敗したら None。

    出力を読むので LC_ALL=C で揃える。訳された見出しを拾えないため。
    """
    try:
        proc = subprocess.run(
            cmd, capture_output=True, text=True, timeout=10, check=True,
            env={**os.environ, "LC_ALL": "C"},
        )
    except (subprocess.SubprocessError, OSError):
        return None
    return proc.stdout


def parse_cpuinfo_model(text: str) -> str | None:
    for line in text.splitlines():
        # x86 は "model name"、Raspberry Pi のような板は "Model" が出る。
        if line.split(":")[0].strip() in ("model name", "Model"):
            return line.split(":", 1)[1].strip() or None
    return None


def parse_lscpu_model(text: str) -> str | None:
    for line in text.splitlines():
        head, sep, tail = line.partition(":")
        if sep and head.strip() == "Model name":
            return tail.strip() or None
    return None


def cpu_model() -> str:
    """CPU のモデル名。キーの一部になる。

    arm の /proc/cpuinfo にはモデル名が載らない。実装者と part 番号しか無いので、
    それを名前に直してくれる lscpu に落とす。ubuntu-24.04-arm では Neoverse-N2 が
    返る。x86 では両方が同じ文字列を返すので、先に読む cpuinfo の側で決まる。
    """
    system = platform.system()
    if system == "Darwin":
        out = _capture(["sysctl", "-n", "machdep.cpu.brand_string"])
        return out.strip() if out and out.strip() else "unknown"
    if system == "Linux":
        try:
            model = parse_cpuinfo_model(Path("/proc/cpuinfo").read_text())
        except OSError:
            model = None
        if model:
            return model
        out = _capture(["lscpu"])
        if out:
            model = parse_lscpu_model(out)
            if model:
                return model
    return "unknown"


def cpu_arch() -> str:
    return platform.machine()


def group_name(env: Environment) -> str:
    """CI のジョブの単位。環境名の最初の区切りまでで、x64-gcc と x64-clang は x64。

    同じ組の環境は同じマシン (runs_on) に載るので、1 本のジョブが両方のコンパイラで
    測れる。稀な CPU モデルに当たった 1 本が、その場で gcc と clang の両方を測る
    ためにこうしている。
    """
    return env.name.split("-", 1)[0]


def groups(envs: list[Environment]) -> dict[str, list[Environment]]:
    """CI で走らせる環境を組ごとに束ねる。runs_on = "self" は入れない。"""
    out: dict[str, list[Environment]] = {}
    for env in envs:
        if env.runs_on == "self":
            continue
        out.setdefault(group_name(env), []).append(env)
    for name, members in out.items():
        runs_on = {e.runs_on for e in members}
        if len(runs_on) != 1:
            raise EnvironmentError_(
                f"環境の組 {name!r} の runs_on が揃っていません: {sorted(runs_on)}"
            )
    return out


def toolchain(env: Environment) -> str:
    """CI がどちらのコンパイラを入れるか。cxx の版の接尾辞を落として見る。

    judge.yml に手で書いていたものを、ここから出すようにした。二重持ちだと
    環境を足したときに片方だけ直して、その環境が黙って走らない。
    """
    base = env.cxx.rsplit("-", 1)[0]
    if base not in TOOLCHAINS:
        raise EnvironmentError_(
            f"{env.name}: cxx {env.cxx!r} のツールチェインが分かりません "
            f"(既知: {', '.join(sorted(TOOLCHAINS))})"
        )
    return TOOLCHAINS[base]
