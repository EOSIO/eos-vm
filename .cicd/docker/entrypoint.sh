#!/usr/bin/env bash
set -eo pipefail
cd /workdir
. ./.cicd/helpers/general.sh
. ./$HELPER_DIR/execute.sh
echo '+++ :hammer: Building eos-vm'
mkdir -p build
execute cd /workdir/build
execute ccache -s
execute cmake -DCMAKE_TOOLCHAIN_FILE=/workdir/.cicd/clang.make -DENABLE_TESTS=ON ..
execute make -j$JOBS
if ${ENABLE_TESTS:-true}; then execute ctest -j$JOBS -V --output-on-failure -T Test; fi
echo '+++ :white_check_mark: Done!'