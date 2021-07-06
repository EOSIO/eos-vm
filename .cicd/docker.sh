#!/bin/bash
set -eo pipefail
echo '--- :cloud: Pulling Base-Image'
export DOCKERFILE="$IMAGE_TAG.dockerfile"
export HASH="$(sha1sum ".cicd/platforms/$DOCKERFILE" | awk '{print $1}')"
export CONTAINER="docker.io/eosio/ci:eos-vm-$IMAGE_TAG-$HASH"
echo "SHA-1 hash of \".cicd/platforms/$DOCKERFILE\" is $HASH."
# base-image
echo "$ docker pull $CONTAINER"
if [[ ! $(docker pull $CONTAINER) ]]; then
    echo '--- :mantelpiece_clock: Building Base-Image'
    cd .cicd/platforms
    echo "$ docker build -t $CONTAINER -f $DOCKERFILE ."
    docker build -t $CONTAINER -f $DOCKERFILE .
    echo "$ docker push $CONTAINER"
    docker push $CONTAINER
    cd ../..
fi
# pass BUILDKITE intrinsics into docker container
if [[ -f $BUILDKITE_ENV_FILE ]]; then
    EVARS=''
    while read -r var; do
        EVARS="$EVARS -e ${var%%=*}"
    done < "$BUILDKITE_ENV_FILE"
fi
# docker run
echo '--- :docker: Running Container'
DOCKER_COMMAND="docker run --rm -v \"$(pwd):/eos-vm\" -w /eos-vm $EVARS -e DOCKER=true $CONTAINER bash -c '$@'"
echo "$ $DOCKER_COMMAND"
eval $DOCKER_COMMAND
