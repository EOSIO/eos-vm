#!/usr/bin/env bash
set -eo pipefail
. ./.cicd/helpers/general.sh
. ./$HELPERS_DIR/execute.sh
. ./$HELPERS_DIR/docker-hash.sh
if [[ $TRAVIS ]]; then
  export JOBS="$(getconf _NPROCESSORS_ONLN)"
  if [[ "$(uname)" == Darwin ]]; then
    ccache -s
    mkdir -p build && cd build && cmake ..
    make -j$JOBS
  else # linux
    execute docker run --rm -v $(pwd):/workdir -v /usr/lib/ccache -v $HOME/.ccache:/opt/.ccache -e JOBS -e CCACHE_DIR=/opt/.ccache $FULL_TAG
  fi
elif [[ $BUILDKITE ]]; then
  execute ./.cicd/generate-base-images.sh
  execute docker run --rm -v $(pwd):/workdir -e JOBS $FULL_TAG
else
  exit 1
fi  
