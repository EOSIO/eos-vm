#!/bin/bash
cd /workdir
. ./.cicd/execute.sh
echo '+++ :hammer: Building eos-vm'
mkdir build
execute cd /workdir/build
execute ccache -s
if [[ $(cat /etc/issue) =~ "Ubuntu 18.04" ]]; then
    execute cmake -DCMAKE_TOOLCHAIN_FILE=/workdir/.cicd/clangu18.make -DENABLE_TESTS=ON ..
else
    execute cmake -DCMAKE_TOOLCHAIN_FILE=/workdir/.cicd/clang.make -DENABLE_TESTS=ON ..
fi
execute make -j$JOBS
echo '+++ :white_check_mark: Done!'