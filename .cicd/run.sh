#!/usr/bin/env bash
set -eo pipefail
. ./.cicd/helpers/general.sh
. ./$HELPERS_DIR/execute.sh

cd $ROOT_DIR

if [[ $(uname) == Darwin ]]; then

    ccache -s
    mkdir -p build
    cd build
    execute cmake ..
    execute make -j$JOBS
    
else # Linux

    . ./$HELPERS_DIR/docker.sh
    . ./$HELPERS_DIR/docker-hash.sh
    
    # Generate Base Images
    execute ./.cicd/generate-base-images.sh

    COMMANDS="echo 'Please provide COMMANDS to run' && exit 1"
    BUILD_COMMANDS="cmake -DCMAKE_TOOLCHAIN_FILE=/workdir/.cicd/helpers/clang.make -DENABLE_TESTS=ON .. && make -j$JOBS &&"
    TEST_COMMANDS="ctest -j$JOBS -V --output-on-failure -T Test"

    # Docker Run Arguments
    ARGS=${ARGS:-"--rm -v $(pwd):/workdir"}
    # Docker Commands
    if [[ $BUILDKITE ]]; then
        ${ENABLE_BUILD:-false} && COMMANDS="$BUILD_COMMANDS"
        ${ENABLE_TEST:-false} && COMMANDS="$COMMANDS $TEST_COMMANDS"
    elif [[ $TRAVIS ]]; then
        ARGS="$ARGS -v /usr/lib/ccache -v $HOME/.ccache:/opt/.ccache -e JOBS -e CCACHE_DIR=/opt/.ccache"
        COMMANDS="ccache -s && $BUILD_COMMANDS $TEST_COMMANDS"
    fi

    # Docker Run
    docker-run "$COMMANDS"

fi

# Buildkite Artifacts
if [[ $BUILDKITE ]]; then
    cd $ROOT_DIR
    execute tar -pczf build.tar.gz build
    execute buildkite-agent artifact upload build.tar.gz
fi