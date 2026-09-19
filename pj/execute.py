"""実行と計測。

プロセスは posix_spawn で起こして wait4 で回収する。子が異常終了したあとでも
ピーク RSS が返るので、MLE や RE のときにもメモリが分かる。
"""

from __future__ import annotations

import json
import os
import platform
import signal
import tempfile
import threading
import time
from dataclasses import dataclass, field
from pathlib import Path

METRICS_PREFIX = "PJ_METRICS "

# 空打ちに与える時間。exec さえ済めばよいので短くてよい。
WARMUP_TIMEOUT_SEC = 2.0


@dataclass(frozen=True)
class RunResult:
    exit_code: int | None
    term_signal: int | None
    timed_out: bool
    wall_ns: int
    max_rss_kb: int
    metrics: dict = field(default_factory=dict)

    @property
    def wall_ms(self) -> int:
        return self.wall_ns // 1_000_000

    @property
    def crashed(self) -> bool:
        return self.term_signal is not None or (self.exit_code or 0) != 0

    @property
    def algo_time_ns(self) -> int | None:
        value = self.metrics.get("algo_time_ns")
        return int(value) if value is not None else None


def _rss_to_kb(ru_maxrss: int) -> int:
    """ru_maxrss の単位はプラットフォームで違う。Linux は KB、macOS はバイト。"""
    if platform.system() == "Darwin":
        return ru_maxrss // 1024
    return ru_maxrss


def parse_metrics(stderr_path: Path) -> dict:
    """stderr から PJ_METRICS の行を拾う。他の出力が混ざっていても平気。"""
    metrics: dict = {}
    try:
        with stderr_path.open("r", errors="replace") as f:
            for line in f:
                if not line.startswith(METRICS_PREFIX):
                    continue
                try:
                    payload = json.loads(line[len(METRICS_PREFIX):])
                except json.JSONDecodeError:
                    continue
                if isinstance(payload, dict):
                    metrics.update(payload)
    except OSError:
        pass
    return metrics


def run(
    binary: Path,
    *,
    stdin_path: Path | None,
    stdout_path: Path,
    stderr_path: Path,
    tle_sec: float,
) -> RunResult:
    """1 ケース走らせる。tle_sec を超えたら kill する。

    メモリは後判定にできるが時間はできない。無限ループの提出があると
    ジョブが埋まるので、その場で打ち切る。
    """
    stdout_path.parent.mkdir(parents=True, exist_ok=True)
    create = os.O_WRONLY | os.O_CREAT | os.O_TRUNC
    file_actions = [
        (os.POSIX_SPAWN_OPEN, 0, str(stdin_path or os.devnull), os.O_RDONLY, 0o644),
        (os.POSIX_SPAWN_OPEN, 1, str(stdout_path), create, 0o644),
        (os.POSIX_SPAWN_OPEN, 2, str(stderr_path), create, 0o644),
    ]

    # posix_spawn は PATH を見ないので絶対パスで渡す。
    exe = str(binary.resolve())
    t0 = time.monotonic_ns()
    pid = os.posix_spawn(exe, [exe], os.environ, file_actions=file_actions)

    lock = threading.Lock()
    state = {"reaped": False, "timed_out": False}

    def on_timeout() -> None:
        with lock:
            if state["reaped"]:
                return
            state["timed_out"] = True
            try:
                os.kill(pid, signal.SIGKILL)
            except ProcessLookupError:
                # ちょうど終わったところ。回収は待っている側がやる。
                pass

    timer = threading.Timer(tle_sec, on_timeout)
    timer.daemon = True
    timer.start()
    try:
        _, status, rusage = os.wait4(pid, 0)
    finally:
        with lock:
            state["reaped"] = True
        timer.cancel()
    wall_ns = time.monotonic_ns() - t0

    if os.WIFSIGNALED(status):
        term_signal: int | None = os.WTERMSIG(status)
        exit_code: int | None = None
    else:
        term_signal = None
        exit_code = os.WEXITSTATUS(status)

    return RunResult(
        exit_code=exit_code,
        term_signal=term_signal,
        timed_out=state["timed_out"],
        wall_ns=wall_ns,
        max_rss_kb=_rss_to_kb(rusage.ru_maxrss),
        metrics=parse_metrics(stderr_path),
    )


def warmup(binary: Path, *, tle_sec: float) -> None:
    """計測の前にバイナリを 1 回空打ちする。

    macOS では新しくコンパイルしたバイナリの初回 exec に 0.3 秒ほどかかる
    (署名の検証)。そのぶんが最初のケースの実時間に乗ると time_max_ms が狂う。
    入力を与えないので提出はすぐ終わる。終わらなくても exec は済んでいるので
    打ち切ってよい。
    """
    with tempfile.TemporaryDirectory() as tmp:
        tmp_path = Path(tmp)
        try:
            run(
                binary,
                stdin_path=None,
                stdout_path=tmp_path / "stdout",
                stderr_path=tmp_path / "stderr",
                tle_sec=min(tle_sec, WARMUP_TIMEOUT_SEC),
            )
        except OSError:
            pass
