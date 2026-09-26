"""計測値の取り出し。"""

from pj import execute
from pj.execute import parse_metrics


def test_reads_the_prefixed_line(tmp_path):
    path = tmp_path / "stderr"
    path.write_text('PJ_METRICS {"algo_time_ns":1234}\n')
    assert parse_metrics(path) == {"algo_time_ns": 1234}


def test_other_output_on_stderr_is_ignored(tmp_path):
    path = tmp_path / "stderr"
    path.write_text(
        "warning: something\n"
        'PJ_METRICS {"algo_time_ns":7}\n'
        "==1234==LeakSanitizer: detected memory leaks\n"
    )
    assert parse_metrics(path) == {"algo_time_ns": 7}


def test_later_lines_win(tmp_path):
    path = tmp_path / "stderr"
    path.write_text('PJ_METRICS {"algo_time_ns":1}\nPJ_METRICS {"algo_time_ns":2}\n')
    assert parse_metrics(path) == {"algo_time_ns": 2}


def test_extra_keys_pass_through(tmp_path):
    path = tmp_path / "stderr"
    path.write_text('PJ_METRICS {"algo_time_ns":1,"rounds":3}\n')
    assert parse_metrics(path) == {"algo_time_ns": 1, "rounds": 3}


def test_broken_json_does_not_raise(tmp_path):
    path = tmp_path / "stderr"
    path.write_text("PJ_METRICS {not json\n")
    assert parse_metrics(path) == {}


def test_missing_file(tmp_path):
    assert parse_metrics(tmp_path / "nope") == {}


# --- ピーク RSS ------------------------------------------------------------


def test_the_report_wins_over_ru_maxrss():
    """posix_spawn した子の ru_maxrss には親のピークが混ざるので信じない。報告を採る。"""
    assert execute.peak_rss_kb({"max_rss_kb": 4096}, 999999) == 4096


def test_without_a_report_it_falls_back_to_the_kernel():
    """異常終了した実行は報告が出ない。そこだけ ru_maxrss を使う。"""
    assert execute.peak_rss_kb({}, 2048) == execute._rss_to_kb(2048)


def test_a_broken_report_falls_back():
    for value in (0, -1, "x", None, True, 1.5):
        assert execute.peak_rss_kb({"max_rss_kb": value}, 2048) == execute._rss_to_kb(
            2048
        ), value


def test_a_fallback_at_or_below_the_parent_peak_is_unknown():
    """Linux の ru_maxrss は max(pj のピーク, 子のピーク)。pj のピーク以下なら子の値は分からない。"""
    kb = execute._rss_to_kb(2048)
    assert execute.peak_rss_kb({}, 2048, parent_peak_kb=kb) == 0
    assert execute.peak_rss_kb({}, 2048, parent_peak_kb=kb - 1) == kb
    assert execute.peak_rss_kb({"max_rss_kb": 7}, 2048, parent_peak_kb=kb) == 7


def test_the_harness_time_and_the_preload_memory_are_merged(tmp_path):
    """base の提出は、ハーネスが時間を、preload がメモリを別々の行で報告する。"""
    path = tmp_path / "stderr"
    path.write_text('PJ_METRICS {"algo_time_ns":5}\nPJ_METRICS {"max_rss_kb":101}\n')
    assert parse_metrics(path) == {"algo_time_ns": 5, "max_rss_kb": 101}


def test_the_child_env_puts_the_preload_first(monkeypatch):
    monkeypatch.setattr(execute, "rss_preload", lambda: "/x/rss.so")
    monkeypatch.setenv("LD_PRELOAD", "/y/other.so")
    assert execute._child_env()["LD_PRELOAD"] == "/x/rss.so:/y/other.so"
    monkeypatch.delenv("LD_PRELOAD")
    assert execute._child_env()["LD_PRELOAD"] == "/x/rss.so"
    monkeypatch.setattr(execute, "rss_preload", lambda: None)
    assert "LD_PRELOAD" not in execute._child_env()


def test_the_parent_rss_does_not_leak_into_the_child(tmp_path):
    """pj が大きなメモリを握っていても、ハーネスを持たない提出のメモリにその分が乗らない。

    Linux では rss_preload が報告する VmHWM で、macOS では ru_maxrss がそのまま正しい。
    """
    import shutil
    import subprocess

    import pytest

    if shutil.which("cc") is None:
        pytest.skip("cc がありません")
    source = tmp_path / "noop.c"
    source.write_text("int main(void) { return 0; }\n")
    binary = tmp_path / "noop"
    subprocess.run(["cc", "-O2", "-o", str(binary), str(source)], check=True)

    blob = bytearray(256 << 20)
    blob[::4096] = b"\x01" * len(range(0, len(blob), 4096))  # 全ページに触って RSS に載せる
    result = execute.run(
        binary,
        stdin_path=None,
        stdout_path=tmp_path / "stdout",
        stderr_path=tmp_path / "stderr",
        tle_sec=10.0,
    )
    assert result.exit_code == 0
    assert 0 < result.max_rss_kb < 64 * 1024, result.max_rss_kb
    if execute.rss_preload() is not None:
        assert result.metrics.get("max_rss_kb") == result.max_rss_kb
    del blob


def test_raise_stack_limit_soft_reaches_hard():
    """スタックの上限を硬い方まで上げる。既定の 8 MB だと深い再帰が落ちる。"""
    import resource

    before = resource.getrlimit(resource.RLIMIT_STACK)
    try:
        execute.raise_stack_limit()
        soft, hard = resource.getrlimit(resource.RLIMIT_STACK)
        assert hard == before[1]
        assert soft == hard
        # 2 回目は何もしない。
        execute.raise_stack_limit()
        assert resource.getrlimit(resource.RLIMIT_STACK) == (soft, hard)
    finally:
        resource.setrlimit(resource.RLIMIT_STACK, before)
