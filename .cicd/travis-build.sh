#!/usr/bin/env bash
set -eo pipefail
. ./.cicd/execute.sh
. ./.cicd/docker-hash.sh

if [[ $TRAVIS ]]; then
  echo "This build is running on TravisCI."
elif [[ $BUILDKITE ]]; then
  echo "This build is running on Buildkite."
else
  echo "Shouldn't get here."
fi  
# if [[ "$(uname)" == Darwin ]]; then
#   ccache -s
#   mkdir -p build && cd build && cmake ..
#   make -j$(getconf _NPROCESSORS_ONLN)
# else # linux
#   execute docker run --rm -v $(pwd):/workdir -v /usr/lib/ccache -v $HOME/.ccache:/opt/.ccache -e CCACHE_DIR=/opt/.ccache $FULL_TAG
# fi
