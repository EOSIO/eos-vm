#!/bin/bash
set -eo pipefail
. ./.cicd/helpers/general.sh
# download artifacts
if [[ "$BUILDKITE" == 'true' ]]; then
    buildkite-agent artifact download build.tar.gz . --step "$1"
    tar -xzf build.tar.gz
fi
COMMANDS="./scripts/test.sh"
if [[ $(uname) == 'Darwin' ]]; then
    echo "$ $COMMANDS"
    $COMMANDS
else # Linux
    MOUNTED_DIR='/workdir'
    ARGS=${ARGS:-"--rm -v $(pwd):$MOUNTED_DIR -w $MOUNTED_DIR"}
    . $HELPERS_DIR/docker-hash.sh
    # base-image
    [[ "$BUILDKITE" == 'true' ]] && $CICD_DIR/generate-base-images.sh
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