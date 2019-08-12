#!/usr/bin/env bash
set -eo pipefail
. ./.cicd/helpers/general.sh
. $HELPERS_DIR/logging.sh

fold-execute mkdir -p $BUILD_DIR

TEST_COMMAND="ctest -j$JOBS --output-on-failure -T Test"

if [[ $(uname) == 'Darwin' ]]; then

    cd $BUILD_DIR
    [[ $TRAVIS == true ]] && fold-execute ccache -s
    fold-execute $TEST_COMMAND

else # Linux

    MOUNTED_DIR='/workdir'
    ARGS=${ARGS:-"--rm -v $(pwd):$MOUNTED_DIR"}

    . $HELPERS_DIR/docker-hash.sh

    PRE_COMMANDS=". $MOUNTED_DIR/.cicd/helpers/logging.sh"
    COMMANDS="cd $MOUNTED_DIR/build && $TEST_COMMAND"

    # Docker Commands
    if [[ $BUILDKITE == true ]]; then
        execute $CICD_DIR/generate-base-images.sh
    elif [[ $TRAVIS == true ]]; then
        ARGS="$ARGS -v /usr/lib/ccache -v $HOME/.ccache:/opt/.ccache -e JOBS -e TRAVIS -e CCACHE_DIR=/opt/.ccache"
        COMMANDS="fold-execute ccache -s && $COMMANDS"
    fi

    # Load BUILDKITE Environment Variables for use in docker run
    if [[ -f $BUILDKITE_ENV_FILE ]]; then
        evars=""
        while read -r var; do
            evars="$evars --env ${var%%=*}"
        done < "$BUILDKITE_ENV_FILE"
    fi
    
    COMMANDS="$PRE_COMMANDS && $COMMANDS"

    # Docker Run with all of the commands we've prepped
    fold-execute eval docker run $ARGS $evars $FULL_TAG bash -c \"$COMMANDS\"

fi