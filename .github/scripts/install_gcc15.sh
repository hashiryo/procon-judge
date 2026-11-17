#!/usr/bin/env bash
# g++-15 を入れる。judge.yml の run と fallback のジョブが使う。入ったら 0、入らなければ 1 を返す。
# 使い方: install_gcc15.sh <.deb の置き場>
#
# 置き場に前の回の .deb (actions/cache で戻したもの) があれば、それを入れて Launchpad には行かない。
# 無ければ PPA (ppa:ubuntu-toolchain-r/test) から落として置き場に残し、そこから入れる。PPA は遅く
# (225 kB/s ほどで 1 回 5 分)、503 を返して落とせない回もあるので、3 回まで間を空けてやり直す。
# PPA の目次が取れなかったときの apt は、g++-15 を別のパッケージ (clang-15 など) で満たして成功を
# 返すので、g++-15 という名前の実体があるかを確かめてから落とす。
# 置き場に残すのは PPA のパッケージ (版に ~ppa を含むもの) だけにする。libc6 などの Ubuntu の
# パッケージまで残すと、ランナーの OS が新しくなったときに古い版へ戻すことになり、apt が断る。
# それらは入れるときに Ubuntu のミラーから取る。
set -u
dir=$1
ok() { command -v g++-15 >/dev/null && g++-15 --version >/dev/null; }

if compgen -G "$dir/g++-15_*.deb" >/dev/null; then
  if sudo apt-get update && sudo apt-get install -y "$dir"/*.deb && ok; then exit 0; fi
  echo "::warning::キャッシュの .deb で g++-15 を入れられませんでした。PPA から入れ直します。キャッシュのキーを替えると直ります"
fi

rm -rf "$dir"
mkdir -p "$dir"
# 落とした .deb だけを選ぶため、先に空にする。やり直しのときは落とし終えた分を使い回す。
sudo apt-get clean
for i in 1 2 3; do
  if sudo add-apt-repository -y -n ppa:ubuntu-toolchain-r/test \
    && sudo apt-get update \
    && apt-cache policy g++-15 | grep -q 'Candidate: [0-9]' \
    && sudo apt-get install -y --download-only g++-15 \
    && compgen -G "/var/cache/apt/archives/g++-15_*.deb" >/dev/null; then
    cp /var/cache/apt/archives/*~ppa*.deb "$dir"/
    if sudo apt-get install -y "$dir"/*.deb && ok; then exit 0; fi
    rm -f "$dir"/*.deb
  fi
  if [ "$i" -lt 3 ]; then
    echo "::warning::PPA から g++-15 を落とせませんでした ($i 回目)。$((30 * i)) 秒待ってやり直します"
    sleep $((30 * i))
  fi
done
exit 1
