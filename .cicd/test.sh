#!/bin/bash
set -eo pipefail
export JOBS=${JOBS:-"$(getconf _NPROCESSORS_ONLN)"}
# download artifacts
if [[ "$BUILDKITE" == 'true' ]]; then
    buildkite-agent artifact download build.tar.gz . --step "$1"
    tar -xzf build.tar.gz
fi
# test
if [[ $(uname) == 'Darwin' ]]; then
    set +e
    echo "$ scripts/test.sh"
    scripts/test.sh
    EXIT_STATUS=$?
else # linux
    . .cicd/docker-hash.sh
    # base-image
    [[ "$BUILDKITE" == 'true' ]] && .cicd/generate-base-images.sh
    # load BUILDKITE intrinsics into docker container
    if [[ -f $BUILDKITE_ENV_FILE ]]; then
        evars=""
        while read -r var; do
            evars="$evars --env ${var%%=*}"
        done < "$BUILDKITE_ENV_FILE"
    fi
    # docker run
    set +e
    echo "$ docker run --rm -v \"$(pwd):/eos-vm\" -w /eos-vm $evars $FULL_TAG bash -c 'scripts/test.sh'"
    eval docker run -v "$(pwd):/eos-vm" -w /eos-vm $evars $FULL_TAG bash -c 'scripts/test.sh'
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
if [[ "$EXIT_STATUS" != '0' ]]; then
    echo "Failing due to non-zero exit status from ctest: $EXIT_STATUS"
    exit $EXIT_STATUS
fi