#!/bin/bash
set -eo pipefail
# download artifacts on host
if [[ "$BUILDKITE" == 'true' && "$DOCKER" != 'true' ]]; then
    echo '--- :arrow_down: Downloading Artifacts'
    buildkite-agent artifact download build.tar.gz . --step "$1"
    tar -xzf build.tar.gz
fi
# test
if [[ ! -z "$IMAGE_TAG" && "$DOCKER" != 'true' ]]; then # linux host > run this script in docker
    .cicd/docker.sh '.cicd/test.sh' $@
    EXIT_STATUS="$?"
else # mac host or linux guest > test
    echo '--- :evergreen_tree: Configuring Environment'
    [[ -z "$JOBS" ]] && export JOBS="$(getconf _NPROCESSORS_ONLN)"
    [[ -d build ]] && cd build
    echo '+++ :microscope: Testing'
    echo 'Enumerating tests...'
    TEST_COUNT="$(ctest -N | grep -i 'Total Tests: ' | cut -d ':' -f 2 | awk '{print $1}')"
    if (( $TEST_COUNT > 0 )); then
        echo "$TEST_COUNT tests found."
        # run tests
        set +e # defer ctest error handling to end
        echo "$ ctest -j $JOBS --output-on-failure -T Test"
        ctest  -j $JOBS --output-on-failure -T Test
        EXIT_STATUS=$?
        echo 'Done running tests.'
    else
        echo '+++ :no_entry: ERROR: No tests registered with ctest!'
        echo 'Exiting...'
        EXIT_STATUS='1'
    fi
fi
# upload artifacts on host
if [[ "$BUILDKITE" == 'true' && "$DOCKER" != 'true' ]]; then
    echo '--- :arrow_up: Uploading Artifacts'
    [[ -d build ]] && cd build
    echo 'Exporting xUnit XML'
    mv -f ./Testing/$(ls ./Testing/ | grep '2' | tail -n 1)/Test.xml test-results.xml
    echo 'Uploading artifacts'
    buildkite-agent artifact upload test-results.xml
    echo 'Done uploading artifacts.'
fi
exit $EXIT_STATUS