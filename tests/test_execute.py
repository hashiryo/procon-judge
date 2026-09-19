"""計測値の取り出し。"""

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
