#!/usr/bin/env bash
set -eo pipefail
cd $( dirname "${BASH_SOURCE[0]}" )/.. # Ensure we're in the repo root and not inside of scripts
. ./.cicd/.helpers

execute docker run -e --rm -v $(pwd):/$PROJECT_NAME $FULL_TAG bash -c " \
cd /$PROJECT_NAME && \
echo '=== Updating Submodules' && \
git submodule update --init --recursive && \
echo '=== BUILDING $PROJECT_NAME' && \
mkdir -p build && cd build && \
mv /\$DCMAKE_TOOLCHAIN_FILE . && \
cmake -DCMAKE_TOOLCHAIN_FILE=\$DCMAKE_TOOLCHAIN_FILE -DENABLE_TESTS=ON .. && \
make -j$(getconf _NPROCESSORS_ONLN) && \
echo '=== COMPLETE' \
"