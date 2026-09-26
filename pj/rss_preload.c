/*
 * 提出の実行に LD_PRELOAD で差し込む共有ライブラリ (Linux だけ)。
 *
 * posix_spawn した子の ru_maxrss には pj 自身のピーク RSS が乗る (execute.peak_rss_kb)。
 * ハーネスを持たない raw の提出でも自分のピークを pj に渡せるよう、プロセスが正常に
 * 終わるときに /proc/self/status の VmHWM を読んで、ハーネスと同じ PJ_METRICS の行を
 * stderr に出す。VmHWM は exec 後の mm だけを見るので、親の分は混ざらない。
 * 異常終了では destructor が呼ばれないので行は出ない。そのときは pj が ru_maxrss に落ちる。
 */
#include <stdio.h>

__attribute__((destructor)) static void pj_report_peak_rss(void) {
  FILE *f = fopen("/proc/self/status", "r");
  if (!f) return;
  char line[256];
  long long kb = -1;
  while (fgets(line, sizeof line, f)) {
    if (sscanf(line, "VmHWM: %lld kB", &kb) == 1) break;
  }
  fclose(f);
  if (kb > 0) fprintf(stderr, "PJ_METRICS {\"max_rss_kb\":%lld}\n", kb);
}
