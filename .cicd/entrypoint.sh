#!/bin/bash
cd /workdir
. ./.cicd/execute.sh
echo '+++ :git: Updating Submodules'
execute git submodule update --init --recursive
echo '+++ :hammer: Building eos-vm'
mkdir build
execute cd /workdir/build
execute ccache -s
execute cmake -DCMAKE_TOOLCHAIN_FILE=/workdir/.cicd/clang.make -DENABLE_TESTS=ON ..
execute make -j $(nproc)
echo '+++ :white_check_mark: Done!'