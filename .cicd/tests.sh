#!/bin/bash
set -eo pipefail
. ./.cicd/helpers/general.sh
# download artifacts
if [[ "$BUILDKITE" == 'true' ]]; then
    buildkite-agent artifact download build.tar.gz . --step "$1"
    tar -xzf build.tar.gz
fi
COMMAND="./scripts/test.sh"
if [[ $(uname) == 'Darwin' ]]; then
    [[ $TRAVIS == true ]] && echo '$ ccache -s' && ccache -s
    echo "$ $COMMAND"
    $COMMAND
else # Linux
    MOUNTED_DIR='/workdir'
    ARGS=${ARGS:-"--rm -v $(pwd):$MOUNTED_DIR -w $MOUNTED_DIR"}
    . $HELPERS_DIR/docker-hash.sh
    # Docker Commands
    if [[ $BUILDKITE == true ]]; then
        $CICD_DIR/generate-base-images.sh
    elif [[ $TRAVIS == true ]]; then
        ARGS="$ARGS -v /usr/lib/ccache -v $HOME/.ccache:/opt/.ccache -e JOBS -e TRAVIS -e CCACHE_DIR=/opt/.ccache"
        COMMANDS="ccache -s && $COMMANDS"
    fi
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