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
    echo "$ scripts/test.sh"
    scripts/test.sh
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
    echo "$ docker run --rm -v \"$(pwd):/eos-vm\" -w /eos-vm $evars $FULL_TAG bash -c 'scripts/test.sh'"
    eval docker run -v "$(pwd):/eos-vm" -w /eos-vm $evars $FULL_TAG bash -c 'scripts/test.sh'
fi