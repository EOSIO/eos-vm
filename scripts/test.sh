#!/bin/bash
set -eo pipefail
echo "+++ $([[ "$BUILDKITE" == 'true' ]] && echo ':evergreen_tree: ')Configuring Environment"
GIT_ROOT="$(dirname $BASH_SOURCE[0])/.."
[[ -z "$JOBS" ]] && export JOBS="$(nproc)"
[[ -z "$TEST" ]] && export TEST="$1"
cd $GIT_ROOT/build
# tests
if [[ -z "$TEST" ]]; then # run all tests
    # count tests
    echo "+++ $([[ "$BUILDKITE" == 'true' ]] && echo ':microscope: ')Running Tests"
    TEST_COUNT="$(ctest -N | grep -i 'Total Tests: ' | cut -d ':' -f 2 | awk '{print $1}')"
    if [[ $TEST_COUNT > 0 ]]; then
        echo "$TEST_COUNT tests found."
        # run tests
        set +e # defer ctest error handling to end
        echo "$ ctest -j $JOBS --output-on-failure -T Test"
        ctest  -j $JOBS --output-on-failure -T Test
        EXIT_STATUS=$?
        echo 'Done running tests.'
    else
        echo "+++ $([[ "$BUILDKITE" == 'true' ]] && echo ':no_entry: ')ERROR: No tests registered with ctest! Exiting..."
        EXIT_STATUS='1'
    fi
else # run specific test
    # ensure test exists
    echo "+++ $([[ "$BUILDKITE" == 'true' ]] && echo ':microscope: ')Running $TEST"
    TEST_COUNT="$(ctest -N -R "^$TEST$" | grep -i 'Total Tests: ' | cut -d ':' -f 2 | awk '{print $1}')"
    if [[ $TEST_COUNT > 0 ]]; then
        echo "$TEST found."
        # run tests
        set +e # defer ctest error handling to end
        echo "$ ctest -R \"^$TEST$\" --output-on-failure -T Test"
        ctest -R "^$TEST$" --output-on-failure -T Test
        EXIT_STATUS=$?
        echo "Done running $TEST."
    else
        echo "+++ $([[ "$BUILDKITE" == 'true' ]] && echo ':no_entry: ')ERROR: No tests matching \"$TEST\" registered with ctest! Exiting..."
        EXIT_STATUS='1'
    fi
fi
exit $EXIT_STATUS