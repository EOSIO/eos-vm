#!/bin/bash
set -eo pipefail
export JOBS=${JOBS:-"$(getconf _NPROCESSORS_ONLN)"}
mkdir build
if [[ $(uname) == 'Darwin' ]]; then
    cd build
    echo '$ cmake -DCMAKE_BUILD_TYPE=Release -DENABLE_TESTS=ON ..'
    cmake -DCMAKE_BUILD_TYPE=Release -DENABLE_TESTS=ON ..
    echo "$ make -j $JOBS"
    make -j $JOBS
else # Linux
    . .cicd/docker-hash.sh
    # base-image
    [[ "$BUILDKITE" == 'true' ]] && .cicd/generate-base-images.sh
    # Load BUILDKITE Environment Variables for use in docker run
    if [[ -f $BUILDKITE_ENV_FILE ]]; then
        evars=""
        while read -r var; do
            evars="$evars --env ${var%%=*}"
        done < "$BUILDKITE_ENV_FILE"
    fi
    # Docker Run with all of the commands we've prepped
    echo "$ docker run --rm -v \"$(pwd):/eos-vm\" $evars $FULL_TAG bash -c \"cd /eos-vm/build && cmake -DCMAKE_TOOLCHAIN_FILE=/eos-vm/.cicd/clang.make -DENABLE_TESTS=ON -DCMAKE_BUILD_TYPE=Release .. && make -j $JOBS\""
    eval docker run --rm -v "$(pwd):/eos-vm" $evars $FULL_TAG bash -c \"cd /eos-vm/build && cmake -DCMAKE_TOOLCHAIN_FILE=/eos-vm/.cicd/clang.make -DENABLE_TESTS=ON -DCMAKE_BUILD_TYPE=Release .. && make -j $JOBS\"
fi
# upload artifacts
if [[ "$BUILDKITE" == 'true' ]]; then
    tar -pczf build.tar.gz build
    buildkite-agent artifact upload build.tar.gz
fi