#!/usr/bin/env bash
set -eo pipefail
. ./.cicd/helpers/general.sh
. ./$HELPERS_DIR/execute.sh

if [[ $(uname) == Darwin ]]; then

    cd $ROOT_DIR
    ccache -s
    mkdir -p build
    cd build
    execute cmake ..
    execute make -j$JOBS
    cd ..
    
else # Linux

    . ./$HELPERS_DIR/docker.sh
    . ./$HELPERS_DIR/docker-hash.sh 'ubuntu-18.04'
    
    # Generate Base Images
    execute ./.cicd/generate-base-images.sh

    BUILD_COMMANDS="mkdir -p /workdir/build && cd /workdir/build && cmake -DCMAKE_TOOLCHAIN_FILE=/workdir/.cicd/helpers/clang.make -DENABLE_TESTS=ON .. && make -j$JOBS"
    TEST_COMMANDS="ctest -j$JOBS -V --output-on-failure -T Test"

    # Docker Run Arguments
    ARGS=${ARGS:-"--rm -v $(pwd):/workdir"}
    # Docker Commands
    if [[ $BUILDKITE ]]; then
        if ${ENABLE_BUILD:-false}; then
            append-to-commands $BUILD_COMMANDS
        fi
        if ${ENABLE_TEST:-false}; then
            append-to-commands $TEST_COMMANDS
        fi
    elif [[ $TRAVIS ]]; then
        ARGS="$ARGS -v /usr/lib/ccache -v $HOME/.ccache:/opt/.ccache -e JOBS -e CCACHE_DIR=/opt/.ccache"
        COMMANDS="ccache -s && $BUILD_COMMANDS && $TEST_COMMANDS"
    fi

    # Docker Run
    docker-run $COMMANDS

fi

# Buildkite Artifacts
if [[ $BUILDKITE ]]; then
    execute tar -pczf build.tar.gz build
    execute buildkite-agent artifact upload build.tar.gz
fi