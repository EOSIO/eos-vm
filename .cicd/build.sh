#!/bin/bash
set -eo pipefail
export JOBS=${JOBS:-"$(getconf _NPROCESSORS_ONLN)"}
mkdir build
if [[ "$(uname)" == 'Darwin' ]]; then
    cd build
    echo '$ cmake -DCMAKE_BUILD_TYPE=Release -DENABLE_TESTS=ON ..'
    cmake -DCMAKE_BUILD_TYPE=Release -DENABLE_TESTS=ON ..
    echo "$ make -j $JOBS"
    make -j $JOBS
    cd ..
else # linux
    . .cicd/docker-hash.sh
    # base-image
    [[ "$BUILDKITE" == 'true' ]] && .cicd/generate-base-images.sh
    # load BUILDKITE intrinsics into docker container
    if [[ -f $BUILDKITE_ENV_FILE ]]; then
        EVARS=''
        while read -r var; do
            EVARS="$EVARS -e ${var%%=*}"
        done < "$BUILDKITE_ENV_FILE"
    fi
    # docker run
    echo "$ docker run --rm -v \"$(pwd):/eos-vm\" $EVARS $FULL_TAG bash -c \"cd build && cmake -DCMAKE_TOOLCHAIN_FILE=/eos-vm/.cicd/clang.make -DENABLE_TESTS=ON -DCMAKE_BUILD_TYPE=Release .. && make -j $JOBS\""
    eval docker run --rm -v "$(pwd):/eos-vm" $EVARS $FULL_TAG bash -c \"cd build && cmake -DCMAKE_TOOLCHAIN_FILE=/eos-vm/.cicd/clang.make -DENABLE_TESTS=ON -DCMAKE_BUILD_TYPE=Release .. && make -j $JOBS\"
fi
# upload artifacts
if [[ "$BUILDKITE" == 'true' ]]; then
    tar -pczf build.tar.gz build
    buildkite-agent artifact upload build.tar.gz
fi