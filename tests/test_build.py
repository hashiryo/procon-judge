"""コンパイル。時間とメモリの山の測り方と、打ち切り。"""

from __future__ import annotations

import sys
import time

from pj import build as build_mod
from pj import environment as env_mod


def build_trivial(tmp_path):
    source = tmp_path / "a.cpp"
    source.write_text("int main() { return 0; }\n")
    return build_mod.build_file(source, env_mod.load("local"), out_dir=tmp_path / "out")


def test_the_compile_reports_its_time_and_memory(tmp_path):
    built = build_trivial(tmp_path)
    assert built.ok, built.log
    assert built.seconds > 0
    # コンパイラのフロントエンドは何もしないソースでも数 MB は使う。
    assert built.rss_kb is not None and built.rss_kb > 1024


def test_the_memory_of_pj_does_not_leak_into_the_compile(tmp_path):
    """Linux では spawn した子の ru_maxrss に親のピーク RSS が乗る。親を 1 段はさんで避ける。"""
    ballast = b"\x01" * (400 << 20)  # 触ったページだけが RSS に乗るので、埋めてから持つ
    built = build_trivial(tmp_path)
    assert len(ballast) == 400 << 20
    assert built.ok, built.log
    assert built.rss_kb is not None and built.rss_kb < 300 * 1024


GRANDCHILD = "import sys, time; time.sleep(2); open(sys.argv[1], 'w').write('x')"
FAKE_COMPILER = (
    "import subprocess, sys, time; "
    f"subprocess.Popen([sys.executable, '-c', {GRANDCHILD!r}, sys.argv[1]]); "
    "time.sleep(30)"
)


def test_the_timeout_stops_the_compiler_and_its_children(tmp_path, monkeypatch):
    # ドライバだけを止めると cc1plus が残る。グループごと止まることを、孫が書くはずの
    # ファイルが書かれないことで見る。
    monkeypatch.setattr(build_mod, "COMPILE_TIMEOUT_SEC", 1)
    marker = tmp_path / "grandchild-finished"
    t0 = time.monotonic()
    built = build_mod._compile(
        [sys.executable, "-c", FAKE_COMPILER, str(marker)], tmp_path / "a.bin", ""
    )
    assert time.monotonic() - t0 < 10
    assert not built.ok
    assert built.log == "コンパイルが 1 秒を超えました"
    time.sleep(2.5)
    assert not marker.exists()


def test_a_missing_compiler_is_a_compile_error_with_the_reason(tmp_path):
    built = build_mod._compile([str(tmp_path / "no-such-compiler")], tmp_path / "a.bin", "")
    assert not built.ok
    assert "no-such-compiler" in built.log


def test_the_summary_shows_the_memory_when_it_is_known():
    built = build_mod.BuildResult(
        ok=True, binary=None, cxxflags="", command=[], seconds=8.74, log="", rss_kb=485 * 1024
    )
    assert build_mod.summary(built) == "8.7s, 485 MB"
    unknown = build_mod.BuildResult(
        ok=True, binary=None, cxxflags="", command=[], seconds=8.74, log=""
    )
    assert build_mod.summary(unknown) == "8.7s"
