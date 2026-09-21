"""出力比較。ここが壊れると記録の意味が変わるので固めておく。"""

from pj.compare import compare_tokens


def write(tmp_path, name, text):
    path = tmp_path / name
    path.write_text(text)
    return path


def test_identical(tmp_path):
    a = write(tmp_path, "a", "1 2 3\n")
    b = write(tmp_path, "b", "1 2 3\n")
    assert compare_tokens(a, b).ok


def test_whitespace_and_newlines_are_ignored(tmp_path):
    a = write(tmp_path, "a", "1\n2\n3\n")
    b = write(tmp_path, "b", "  1 2   3  \n\n")
    assert compare_tokens(a, b).ok


def test_missing_trailing_newline(tmp_path):
    a = write(tmp_path, "a", "1 2 3")
    b = write(tmp_path, "b", "1 2 3\n")
    assert compare_tokens(a, b).ok


def test_differing_token(tmp_path):
    a = write(tmp_path, "a", "1 9 3\n")
    b = write(tmp_path, "b", "1 2 3\n")
    result = compare_tokens(a, b)
    assert not result.ok
    assert "token 1" in result.detail


def test_extra_token(tmp_path):
    a = write(tmp_path, "a", "1 2 3 4\n")
    b = write(tmp_path, "b", "1 2 3\n")
    assert not compare_tokens(a, b).ok


def test_missing_token(tmp_path):
    a = write(tmp_path, "a", "1 2\n")
    b = write(tmp_path, "b", "1 2 3\n")
    assert not compare_tokens(a, b).ok


def test_both_empty(tmp_path):
    a = write(tmp_path, "a", "")
    b = write(tmp_path, "b", "\n")
    assert compare_tokens(a, b).ok


def test_numbers_compare_as_text(tmp_path):
    # トークン比較なので 1 と 1.0 は違う。誤差を許したいなら compare.kind = "float"。
    a = write(tmp_path, "a", "1.0\n")
    b = write(tmp_path, "b", "1\n")
    assert not compare_tokens(a, b).ok


# --- float ------------------------------------------------------------------

from pj.compare import compare, compare_float


def test_float_accepts_absolute_error(tmp_path):
    a = write(tmp_path, "a", "1.00000001 x\n")
    b = write(tmp_path, "b", "1.0 x\n")
    assert compare_float(a, b, abs_tol=1e-6, rel_tol=0).ok
    assert not compare_float(a, b, abs_tol=1e-9, rel_tol=0).ok


def test_float_accepts_relative_error(tmp_path):
    a = write(tmp_path, "a", "1000000.5\n")
    b = write(tmp_path, "b", "1000000.0\n")
    # 絶対誤差は 0.5 で外れるが、相対誤差は 5e-7 で収まる。
    assert compare_float(a, b, abs_tol=1e-6, rel_tol=1e-6).ok
    assert not compare_float(a, b, abs_tol=1e-6, rel_tol=1e-9).ok


def test_float_compares_non_numbers_as_strings(tmp_path):
    a = write(tmp_path, "a", "Yes 1.5\n")
    b = write(tmp_path, "b", "No 1.5\n")
    result = compare_float(a, b, abs_tol=1, rel_tol=1)
    assert not result.ok
    assert "token 0" in result.detail


def test_float_rejects_nan_and_a_different_count(tmp_path):
    a = write(tmp_path, "a", "nan\n")
    b = write(tmp_path, "b", "1.0\n")
    assert not compare_float(a, b, abs_tol=1, rel_tol=1).ok
    a = write(tmp_path, "a2", "1.0 2.0\n")
    b = write(tmp_path, "b2", "1.0\n")
    assert "token count" in compare_float(a, b, abs_tol=1, rel_tol=1).detail


def test_compare_dispatches_float(tmp_path):
    a = write(tmp_path, "a", "0.30000000000000004\n")
    b = write(tmp_path, "b", "0.3\n")
    ok = compare(
        "float", input_path=a, actual_path=a, expected_path=b, abs_tol=1e-9, rel_tol=1e-9
    )
    assert ok.ok
    exact = compare("tokens", input_path=a, actual_path=a, expected_path=b)
    assert not exact.ok
