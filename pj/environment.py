"""environments.toml の読み込みと、走っているマシンの素性。"""

from __future__ import annotations

import platform
import subprocess
import tomllib
from dataclasses import dataclass
from pathlib import Path

from .paths import ENVIRONMENTS_TOML


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


def cpu_model() -> str:
    system = platform.system()
    if system == "Darwin":
        try:
            proc = subprocess.run(
                ["sysctl", "-n", "machdep.cpu.brand_string"],
                capture_output=True, text=True, timeout=10, check=True,
            )
            return proc.stdout.strip()
        except (subprocess.SubprocessError, OSError):
            return "unknown"
    if system == "Linux":
        try:
            for line in Path("/proc/cpuinfo").read_text().splitlines():
                # x86 は "model name"、arm は "Model" が出る。
                if line.split(":")[0].strip() in ("model name", "Model"):
                    return line.split(":", 1)[1].strip()
        except OSError:
            pass
    return "unknown"


def cpu_arch() -> str:
    return platform.machine()
