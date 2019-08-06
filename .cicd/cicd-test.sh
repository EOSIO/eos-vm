#!/usr/bin/env bash
set -eo pipefail
. ./.cicd/helpers/general.sh
. ./$HELPERS_DIR/execute.sh
. ./$HELPERS_DIR/docker-hash.sh
if [[ $(uname) == Darwin ]]; then
  ccache -s
  mkdir -p build && cd build && cmake ..
  make -j$JOBS
else
  if [[ $TRAVIS ]]; then
    execute docker run --rm -v $(pwd):/workdir -v /usr/lib/ccache -v $HOME/.ccache:/opt/.ccache -e JOBS -e CCACHE_DIR=/opt/.ccache $FULL_TAG
  elif [[ $BUILDKITE ]]; then
    execute ./.cicd/generate-base-images.sh
    execute docker run --rm -v $(pwd):/workdir -e JOBS $FULL_TAG
  else
    exit 1
  fi
fi

# Buildkite Artifacts
if [[ $BUILDKITE ]]; then
  tar -pczf build.tar.gz build && buildkite-agent artifact upload build.tar.gz
fi