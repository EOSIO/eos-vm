#!/usr/bin/env bash
set -eo pipefail
. ./.cicd/helpers/general.sh
. ./$HELPERS_DIR/execute.sh

execute mkdir -p $BUILD_DIR

if [[ $(uname) == Darwin ]]; then

    MAC_CMAKE="cmake -DCMAKE_BUILD_TYPE=Release .."
    MAC_MAKE="make -j$JOBS"
    MAC_TEST="ctest -j$JOBS --output-on-failure -T Test"
    cd $BUILD_DIR
    execute ccache -s
    if [[ $ENABLE_BUILD == true ]] || [[ $TRAVIS == true ]]; then
        execute $MAC_CMAKE
        execute $MAC_MAKE
    fi
    if [[ $BUILDKITE == true ]]; then
        [[ $ENABLE_TEST == true ]] && execute $MAC_TEST
    elif [[ $TRAVIS == true ]]; then
        execute mkdir -p wasms
        #execute $MAC_TEST
    fi

else # Linux

    MOUNTED_DIR='/workdir'

    . ./$HELPERS_DIR/docker.sh
    . ./$HELPERS_DIR/docker-hash.sh

    BUILD_COMMANDS="cd $MOUNTED_DIR/build && cmake -DCMAKE_TOOLCHAIN_FILE=$MOUNTED_DIR/.cicd/helpers/clang.make -DENABLE_TESTS=ON -DCMAKE_BUILD_TYPE=Release .. && make -j$JOBS"
    TEST_COMMANDS="cd $MOUNTED_DIR/build && ctest -j$JOBS --output-on-failure -T Test"

    # Docker Run Arguments
    ARGS=${ARGS:-"--rm -v $(pwd):/workdir"}
    # Docker Commands
    if [[ $BUILDKITE == true ]]; then
        # Generate Base Images
        execute ./.cicd/generate-base-images.sh
        [[ ! -d $ROOT_DIR/build/wasms ]] && execute git clone git@github.com:EOSIO/eos-vm-test-wasms.git $ROOT_DIR/build/wasms # support for private wasm repo (contact Bucky)
        [[ $ENABLE_BUILD == true ]] && append-to-commands $BUILD_COMMANDS
        [[ $ENABLE_TEST == true ]] && append-to-commands $TEST_COMMANDS
    elif [[ $TRAVIS == true ]]; then
        execute mkdir $ROOT_DIR/build/wasms
        ARGS="$ARGS -v /usr/lib/ccache -v $HOME/.ccache:/opt/.ccache -e JOBS -e CCACHE_DIR=/opt/.ccache"
        COMMANDS="ccache -s && $BUILD_COMMANDS"
    fi

    # Docker Run
    docker-run $COMMANDS

fi