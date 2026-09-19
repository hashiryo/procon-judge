// 問題一覧。data/index.json だけを読む。
"use strict";

function el(tag, text, className) {
  const node = document.createElement(tag);
  if (text !== undefined && text !== null) node.textContent = String(text);
  if (className) node.className = className;
  return node;
}

function stamp(iso) {
  return iso ? iso.replace("T", " ").replace("Z", " UTC") : "";
}

function link(href, text) {
  const a = el("a", text);
  a.href = href;
  return a;
}

async function main() {
  const data = await (await fetch(document.body.dataset.src)).json();
  document.getElementById("sub").textContent =
    "問題 " + data.problems.length +
    " / 記録 " + data.record_count +
    " / " + stamp(data.generated_at) + " 生成";

  const body = document.getElementById("rows");
  if (data.problems.length === 0) {
    document.getElementById("wrap").replaceChildren(
      el("p", "まだ問題がありません。", "empty")
    );
    return;
  }
  for (const p of data.problems) {
    const tr = el("tr");
    const first = el("td");
    first.append(link("problems/" + encodeURIComponent(p.id) + ".html", p.id));
    if (p.title) first.append(el("div", p.title, "dim"));
    tr.append(first);
    tr.append(el("td", p.source, "dim"));
    tr.append(el("td", p.submissions, "n"));
    const cover = el("td", p.measured + " / " + (p.measured + p.pending), "n");
    if (p.pending > 0) cover.classList.add("dim");
    tr.append(cover);
    tr.append(el("td", p.records, "n"));
    tr.append(el("td", stamp(p.updated), "dim"));
    body.append(tr);
  }
}

main().catch((e) => {
  document.getElementById("sub").textContent = "読み込みに失敗しました: " + e;
});
