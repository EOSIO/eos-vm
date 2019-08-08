#!/usr/bin/env bash
set -eo pipefail
. ./.cicd/helpers/general.sh
. ./$HELPERS_DIR/execute.sh

echo $ENABLE_BUILD

execute mkdir -p $ROOT_DIR/build

if [[ $(uname) == Darwin ]]; then
    MAC_CMAKE="cmake -DCMAKE_BUILD_TYPE=Release .."
    MAC_MAKE="make -j$JOBS"
    MAC_TEST="ctest -j$JOBS --output-on-failure -T Test"
    cd $ROOT_DIR
    ccache -s
    cd build
    if [[ $BUILDKITE ]]; then
        if [[ $ENABLE_BUILD ]]; then
            execute $MAC_CMAKE
            execute $MAC_MAKE
        elif [[ $ENABLE_TEST ]]; then
            execute $MAC_TEST
        fi
    elif [[ $TRAVIS ]]; then
        execute mkdir -p wasms
        execute $MAC_CMAKE
        execute $MAC_MAKE
        #execute $MAC_TEST
    fi

else # Linux

    . ./$HELPERS_DIR/docker.sh
    . ./$HELPERS_DIR/docker-hash.sh

    BUILD_COMMANDS="cd /workdir/build && cmake -DCMAKE_TOOLCHAIN_FILE=/workdir/.cicd/helpers/clang.make -DENABLE_TESTS=ON .. && make -j$JOBS"
    TEST_COMMANDS="cd /workdir/build && ctest -j$JOBS --output-on-failure -T Test"

    # Docker Run Arguments
    ARGS=${ARGS:-"--rm -v $(pwd):/workdir"}
    # Docker Commands
    if [[ $BUILDKITE ]]; then
        # Generate Base Images
        execute ./.cicd/generate-base-images.sh
        [[ ! -d $ROOT_DIR/build/wasms ]] && execute git clone git@github.com:EOSIO/eos-vm-test-wasms.git $ROOT_DIR/build/wasms # support for private wasm repo (contact Bucky)
        [[ $ENABLE_BUILD ]] && append-to-commands $BUILD_COMMANDS
        [[ $ENABLE_TEST ]] && append-to-commands $TEST_COMMANDS
    elif [[ $TRAVIS ]]; then
        execute mkdir $ROOT_DIR/build/wasms
        ARGS="$ARGS -v /usr/lib/ccache -v $HOME/.ccache:/opt/.ccache -e JOBS -e CCACHE_DIR=/opt/.ccache"
        COMMANDS="ccache -s && $BUILD_COMMANDS"
    fi

    # Docker Run
    docker-run $COMMANDS

fi