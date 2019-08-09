export ROOT_DIR=$( dirname "${BASH_SOURCE[0]}" )/../..
export BUILD_DIR=$( dirname "${BASH_SOURCE[0]}" )/../../build
export CICD_DIR=$( dirname "${BASH_SOURCE[0]}" )/..
export HELPERS_DIR=$( dirname "${BASH_SOURCE[0]}" )
export JOBS=${JOBS:-"$(getconf _NPROCESSORS_ONLN)"}

export ENABLE_BUILD=${ENABLE_BUILD:-false}
export ENABLE_TEST=${ENABLE_TEST:-false}