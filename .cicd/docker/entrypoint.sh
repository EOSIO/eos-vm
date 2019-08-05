#!/usr/bin/env bash
set -eo pipefail
cd /workdir
. ./.cicd/helpers/general.sh
. ./$HELPERS_DIR/execute.sh
echo '+++ :hammer: Building eos-vm'
mkdir -p build
execute cd /workdir/build
execute ccache -s
execute cmake -DCMAKE_TOOLCHAIN_FILE=/workdir/.cicd/helpers/clang.make -DENABLE_TESTS=ON ..
execute make -j$JOBS
${ENABLE_TESTS:-true} && execute ctest -j$JOBS -V --output-on-failure -T Test
echo '+++ :white_check_mark: Done!'