#!/bin/bash
set -eo pipefail
. .cicd/helpers/general.sh
mkdir build
if [[ $(uname) == 'Darwin' ]]; then
    cd build
    echo '$ cmake -DCMAKE_BUILD_TYPE=Release -DENABLE_TESTS=ON ..'
    cmake -DCMAKE_BUILD_TYPE=Release -DENABLE_TESTS=ON ..
    echo "$ make -j $JOBS"
    make -j $JOBS
else # Linux
    MOUNTED_DIR='/workdir'
    ARGS=${ARGS:-"--rm -v $(pwd):$MOUNTED_DIR"}
    . .cicd/helpers/docker-hash.sh
    COMMANDS="cd $MOUNTED_DIR/build && cmake -DCMAKE_TOOLCHAIN_FILE=$MOUNTED_DIR/.cicd/helpers/clang.make -DENABLE_TESTS=ON -DCMAKE_BUILD_TYPE=Release .. && make -j$JOBS"
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
    echo "$ docker run $ARGS $evars $FULL_TAG bash -c \"$COMMANDS\""
    eval docker run $ARGS $evars $FULL_TAG bash -c \"$COMMANDS\"
fi
# upload artifacts
if [[ "$BUILDKITE" == 'true' ]]; then
    tar -pczf build.tar.gz build
    buildkite-agent artifact upload build.tar.gz
fi