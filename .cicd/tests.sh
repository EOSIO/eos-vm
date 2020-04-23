#!/bin/bash
set -eo pipefail
. .cicd/helpers/general.sh
# download artifacts
if [[ "$BUILDKITE" == 'true' ]]; then
    buildkite-agent artifact download build.tar.gz . --step "$1"
    tar -xzf build.tar.gz
fi
COMMANDS="./scripts/test.sh"
if [[ $(uname) == 'Darwin' ]]; then
    set +e
    echo "$ $COMMANDS"
    $COMMANDS
    EXIT_STATUS=$?
else # Linux
    MOUNTED_DIR='/workdir'
    ARGS=${ARGS:-"--rm -v $(pwd):$MOUNTED_DIR -w $MOUNTED_DIR"}
    . .cicd/helpers/docker-hash.sh
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
    set +e
    echo "$ docker run $ARGS $evars $FULL_TAG bash -c \"$COMMANDS\""
    eval docker run $ARGS $evars $FULL_TAG bash -c \"$COMMANDS\"
    EXIT_STATUS=$?
fi
# buildkite
if [[ "$BUILDKITE" == 'true' ]]; then
    cd build
    # upload artifacts
    echo '+++ :arrow_up: Uploading Artifacts'
    echo 'Exporting xUnit XML'
    mv -f ./Testing/$(ls ./Testing/ | grep '2' | tail -n 1)/Test.xml test-results.xml
    echo 'Uploading artifacts'
    buildkite-agent artifact upload test-results.xml
    echo 'Done uploading artifacts.'
fi
# re-throw
if [[ "$EXIT_STATUS" != 0 ]]; then
    echo "Failing due to non-zero exit status from ctest: $EXIT_STATUS"
    exit $EXIT_STATUS
fi