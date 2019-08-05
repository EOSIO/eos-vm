#!/usr/bin/env bash
set -eo pipefail
cd /workdir
. ./.cicd/helpers/general.sh
. ./$HELPERS_DIR/execute.sh
echo '+++ :hammer: Building eos-vm'
mkdir -p build
execute cd /workdir/build
execute ccache -s
[[ ! $(cat /etc/issue) =~ "Ubuntu 18.04" ]] && CMAKE_EXTRAS="-DCMAKE_TOOLCHAIN_FILE=/workdir/.cicd/helpers/clang.make"
execute cmake $CMAKE_EXTRAS -DENABLE_TESTS=ON ..
execute make -j$JOBS
if ${ENABLE_TESTS:-true}; then execute ctest -j$JOBS -V --output-on-failure -T Test; fi
echo '+++ :white_check_mark: Done!'