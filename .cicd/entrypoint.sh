#!/bin/bash
cd /workdir
. ./.cicd/execute.sh
echo '+++ :hammer: Building eos-vm'
mkdir build
execute cd /workdir/build
execute ccache -s
execute cmake -DCMAKE_TOOLCHAIN_FILE=/workdir/.cicd/clang.make -DENABLE_TESTS=ON ..
execute make -j$JOBS
echo '+++ :white_check_mark: Done!'