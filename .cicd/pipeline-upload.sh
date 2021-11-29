#!/bin/bash
set -eo pipefail

echo '+++ :evergreen_tree: Configuring Environment'
if [[ -z "$TIMEOUT" ]]; then # use defaults
    [[ -z "$BUILD_TIMEOUT" ]] && export BUILD_TIMEOUT='60'
    [[ -z "$TEST_TIMEOUT" ]] && export TEST_TIMEOUT='10'
else # user override
    [[ -z "$BUILD_TIMEOUT" ]] && export BUILD_TIMEOUT="$TIMEOUT"
    [[ -z "$TEST_TIMEOUT" ]] && export TEST_TIMEOUT="$TIMEOUT"
fi # the TIMEOUT=${TIMEOUT:-45} syntax does not work in the pipeline.yml for timeout values for some reason

export SKIP_MACOS_10_14="true"
export MACOS_10_14_TAG="eos-vm-macos-10.14-$(sha1sum ./.cicd/platforms/macos-10.14.sh | awk '{print $1}')"
export MACOS_10_15_TAG="eos-vm-macos-10.15-$(sha1sum ./.cicd/platforms/macos-10.15.sh | awk '{print $1}')"

echo '+++ :pipeline_upload: Deploying Pipeline Steps'
buildkite-agent pipeline upload .cicd/pipeline.yml
