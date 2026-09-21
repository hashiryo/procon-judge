// 1 問題の順位表。data/problems/<id>.json だけを読む。
"use strict";

function el(tag, text, className) {
  const node = document.createElement(tag);
  if (text !== undefined && text !== null) node.textContent = String(text);
  if (className) node.className = className;
  return node;
}

// 列幅に収める。秒と UTC は落として、元の値は title に残す。
function stamp(iso) {
  return iso ? iso.slice(0, 16).replace("T", " ") : "";
}

function ms(ns) {
  return ns === null || ns === undefined ? "-" : (ns / 1e6).toFixed(2) + " ms";
}

function mb(kb) {
  return kb ? (kb / 1024).toFixed(1) + " MB" : "-";
}

function bytes(n) {
  if (n === null || n === undefined) return "-";
  return n < 1024 ? n + " B" : (n / 1024).toFixed(1) + " KB";
}

// 既定の並びは algo の最大ケース。計測区間の外の I/O と整形を含まないので、
// 実装どうしを比べるならこちら。algo の無い記録は実時間で代用する。
function speed(row) {
  return row.algo_ns === null || row.algo_ns === undefined
    ? row.wall_ms * 1e6
    : row.algo_ns;
}

function linkCell(text, href, className) {
  const td = el("td", null, className);
  if (!href) {
    td.textContent = text;
    return td;
  }
  const a = el("a", text);
  a.href = href;
  td.append(a);
  return td;
}

// 測ってからソースが変わった行に付ける。順位には入れない。
// 何が変わったかは記録から分かるので title に出す。古い記録には無い。
function chip(reason) {
  const span = el("span", "参考", "chip");
  span.title =
    "測ってからソースが変わっています" +
    (reason ? " (" + reason + ")" : "") +
    "。次にこの CPU モデルのランナーが当たったら測り直します";
  return span;
}

// 提出ページ。data/problems/<id>.json の pages に、サイトのルートからのパスで入る。
function pageHref(submission) {
  const page = DATA.pages && DATA.pages[submission];
  return page ? "../" + page : null;
}

// 提出はどれも submissions/ の下にある。全行で同じなので列では落とす。
// 元のパスは title と、ソースの列のリンクに残る。
function shortName(path) {
  return path.startsWith("submissions/") ? path.slice(12) : path;
}

// 提出の列。名前だけを詰めて、後ろの印が押し出されないようにする。flex は
// td ではなく中の div に掛ける。td を flex にすると table-cell でなくなり、
// 行の高さに伸びなくなって、この列だけ罫線がずれる。
function nameCell(path, className, mark) {
  const td = el("td");
  td.title = path;
  const wrap = el("div", null, "namecell");
  const href = pageHref(path);
  const name = el(href ? "a" : "span", shortName(path), className);
  if (href) name.href = href;
  wrap.append(name);
  if (mark) wrap.append(mark);
  td.append(wrap);
  return td;
}

function statusCell(row) {
  const td = el("td", row.status, "st st-" + row.status);
  if (row.failed && row.failed.name) {
    let text = " " + row.failed.name;
    // WA と RE は最後まで走らせるので、落ちたケースが複数あることがある。
    const others = (row.failed_cases || []).filter((n) => n !== row.failed.name);
    if (others.length) text += " +" + others.length;
    td.append(el("span", text, "dim"));
  }
  if (row.failed && row.failed.detail) td.title = row.failed.detail;
  return td;
}

// その記録を測ったコミットのソース。いまの main ではない。
function sourceHref(row) {
  if (!DATA.repo || !row.judge_sha) return null;
  return (
    DATA.repo +
    "/blob/" +
    row.judge_sha +
    "/problems/" +
    encodeURIComponent(DATA.id) +
    "/" +
    row.submission
  );
}

const COLUMNS = [
  {
    // 幅を決めない列は残りをもらう。提出のパスがいちばん長い。
    id: "submission",
    label: "提出",
    text: true,
    value: (r) => r.submission,
    cell: (r) =>
      nameCell(r.submission, "mono name", r.current === false ? chip(r.reason) : null),
  },
  {
    id: "status",
    label: "状態",
    text: true,
    width: "180px",
    value: (r) => r.status,
    cell: statusCell,
  },
  {
    id: "algo",
    label: "algo 最大",
    width: "110px",
    value: speed,
    // 打ち切られた実行の algo は通ったケースまでの値でしかない。数字は残すが、
    // 比べる根拠には見えないように落とす。
    cell: (r) => el("td", ms(r.algo_ns), r.status === "AC" ? "n" : "n dim"),
  },
  {
    id: "wall",
    label: "実時間 最大",
    width: "130px",
    value: (r) => r.wall_ms,
    cell: (r) => el("td", r.wall_ms + " ms", "n"),
  },
  {
    id: "rss",
    label: "メモリ",
    width: "95px",
    value: (r) => r.rss_kb,
    cell: (r) => el("td", mb(r.rss_kb), "n"),
  },
  {
    id: "source",
    label: "ソース",
    width: "90px",
    value: (r) => r.source_bytes,
    cell: (r) => linkCell(bytes(r.source_bytes), sourceHref(r), "n"),
  },
  {
    id: "binary",
    label: "バイナリ",
    width: "100px",
    value: (r) => (r.binary_bytes === null ? -1 : r.binary_bytes),
    cell: (r) => el("td", bytes(r.binary_bytes), "n"),
  },
  {
    id: "samples",
    label: "標本",
    width: "72px",
    value: (r) => r.samples,
    cell: (r) => el("td", r.samples, "n"),
  },
  {
    id: "measured",
    label: "計測 (UTC)",
    text: true,
    width: "160px",
    value: (r) => r.timestamp,
    cell: (r) => {
      const td = el("td", stamp(r.timestamp), "dim");
      td.title = r.timestamp;
      return td;
    },
  },
];

let DATA = null;
let sortBy = "algo";
let ascending = true;

function envs() {
  return [...new Set(DATA.combos.map((c) => c.env))];
}

function modelsOf(env) {
  return DATA.combos.filter((c) => c.env === env).map((c) => c.cpu_model);
}

function fill(select, values, keep) {
  select.replaceChildren();
  for (const v of values) {
    const option = el("option", v);
    option.value = v;
    select.append(option);
  }
  if (keep && values.includes(keep)) select.value = keep;
}

function compare(a, b, column) {
  const x = column.value(a);
  const y = column.value(b);
  return column.text ? String(x).localeCompare(String(y)) : x - y;
}

function sortRows(rows) {
  const column = COLUMNS.find((c) => c.id === sortBy);
  const sign = ascending ? 1 : -1;
  return [...rows].sort((a, b) => {
    // 参考は今のソースで測ったものではない。順位に混ぜず下にまとめる。
    const fresh = (a.current === false ? 1 : 0) - (b.current === false ? 1 : 0);
    if (fresh !== 0) return fresh;
    // どの列で並べても、通した実装が先。通っていないものを混ぜない。
    const passed = (a.status === "AC" ? 0 : 1) - (b.status === "AC" ? 0 : 1);
    if (passed !== 0) return passed;
    return sign * compare(a, b, column) || a.submission.localeCompare(b.submission);
  });
}

function renderHead() {
  const cols = document.getElementById("cols");
  cols.replaceChildren();
  for (const column of COLUMNS) {
    const col = document.createElement("col");
    if (column.width) col.style.width = column.width;
    cols.append(col);
  }

  const tr = document.getElementById("head");
  tr.replaceChildren();
  for (const column of COLUMNS) {
    const th = el("th", column.label, column.text ? "sortable" : "n sortable");
    // 印は出ていないときも場所を取る。押すたびに見出しがずれないように。
    th.append(el("span", column.id === sortBy ? (ascending ? "▲" : "▼") : "", "mark"));
    th.addEventListener("click", () => {
      if (sortBy === column.id) ascending = !ascending;
      else {
        sortBy = column.id;
        ascending = true;
      }
      renderHead();
      render();
    });
    tr.append(th);
  }
}

function render() {
  const env = document.getElementById("env").value;
  const model = document.getElementById("model").value;
  const combo = DATA.combos.find((c) => c.env === env && c.cpu_model === model);
  document.getElementById("combo-meta").replaceChildren(
    el("span", combo ? combo.compiler_version : ""),
    el("span", DATA.cxxflags[env] || "", "flags mono")
  );

  const rows = DATA.rows.filter((r) => r.env === env && r.cpu_model === model);
  const measured = new Set(rows.map((r) => r.submission));
  const missing = DATA.submissions.filter((s) => !measured.has(s));
  const stale = rows.filter((r) => r.current === false).length;

  const body = document.getElementById("rows");
  body.replaceChildren();
  for (const row of sortRows(rows)) {
    const tr = el("tr", null, row.current === false ? "stale" : null);
    for (const column of COLUMNS) tr.append(column.cell(row));
    body.append(tr);
  }
  for (const name of missing) body.append(missingLine(name));

  // 参考と未計測はどちらもランナー待ち。表のどこまでが現行かをここで出す。
  document.getElementById("counts").replaceChildren(
    el("span", "現行 " + (rows.length - stale)),
    el("span", "参考 " + stale),
    el("span", "未計測 " + missing.length)
  );
}

// 記録が無い提出。行ごと落とすと、まだそのランナーが当たっていないだけなのか、
// 提出が無いのかを見分けられない。行は残して値だけ空にする。
function missingLine(name) {
  const tr = el("tr", null, "missing");
  for (const column of COLUMNS) {
    if (column.id === "submission") {
      tr.append(nameCell(name, "mono dim name"));
    } else if (column.id === "status") {
      tr.append(el("td", "未計測", "dim"));
    } else {
      tr.append(el("td", "-", column.text ? "dim" : "n dim"));
    }
  }
  return tr;
}

// 判定サイトのテストデータでない問題の注意書き。自作なら作るファイルへ飛べる。
function renderNote() {
  if (!DATA.note) return;
  const p = el("p", DATA.note, DATA.caution ? "notice warn" : "notice");
  const files = [
    ["ジェネレータ", DATA.generator],
    ["参照実装", DATA.reference],
  ];
  let first = true;
  for (const [label, rel] of files) {
    if (!rel || !DATA.repo || !DATA.judge_sha) continue;
    p.append(first ? " " : " / ");
    first = false;
    const a = el("a", label);
    a.href = DATA.repo + "/blob/" + DATA.judge_sha + "/" + rel;
    p.append(a);
  }
  document.getElementById("meta").after(p);
}

async function main() {
  DATA = await (await fetch(document.body.dataset.src)).json();
  document.getElementById("title").textContent = DATA.title || DATA.id;
  document.getElementById("sub").textContent = DATA.id;
  const meta = [
    el("span", "取得元 " + (DATA.source_label || DATA.source)),
    el("span", "比較 " + DATA.compare),
    el("span", "制限 " + DATA.tle_sec + " 秒 / " + DATA.mle_mb + " MB"),
    el("span", "ケース " + DATA.case_count),
    el("span", "提出 " + DATA.submissions.length),
  ];
  if (DATA.url) {
    const original = el("span");
    const a = el("a", "原題");
    a.href = DATA.url;
    original.append(a);
    meta.push(original);
  }
  document.getElementById("meta").replaceChildren(...meta);
  renderNote();

  if (DATA.combos.length === 0) {
    document.getElementById("wrap").replaceChildren(
      el("p", "まだ記録がありません。", "empty")
    );
    document.getElementById("controls").style.display = "none";
    return;
  }

  const envSelect = document.getElementById("env");
  const modelSelect = document.getElementById("model");
  fill(envSelect, envs());
  envSelect.value = DATA.combos[0].env;
  fill(modelSelect, modelsOf(envSelect.value));
  modelSelect.value = DATA.combos[0].cpu_model;

  envSelect.addEventListener("change", () => {
    // 環境を変えても、同じ CPU モデルがあれば選び直さない。
    fill(modelSelect, modelsOf(envSelect.value), modelSelect.value);
    render();
  });
  modelSelect.addEventListener("change", render);
  renderHead();
  render();
}

main().catch((e) => {
  document.getElementById("sub").textContent = "読み込みに失敗しました: " + e;
});
