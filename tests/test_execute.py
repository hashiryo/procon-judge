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


def test_the_harness_report_wins_over_ru_maxrss():
    """posix_spawn した子の ru_maxrss には親のピークが混ざるので信じない。"""
    assert execute.peak_rss_kb({"max_rss_kb": 4096}, 999999) == 4096


def test_without_a_report_it_falls_back_to_the_kernel():
    """打ち切られた実行はハーネスが報告できない。そこだけ ru_maxrss を使う。"""
    assert execute.peak_rss_kb({}, 2048) == execute._rss_to_kb(2048)


def test_a_broken_report_falls_back():
    for value in (0, -1, "x", None, True, 1.5):
        assert execute.peak_rss_kb({"max_rss_kb": value}, 2048) == execute._rss_to_kb(
            2048
        ), value
