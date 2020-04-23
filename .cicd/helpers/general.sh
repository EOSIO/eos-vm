export ROOT_DIR=$( dirname "${BASH_SOURCE[0]}" )/../..
export JOBS=${JOBS:-"$(getconf _NPROCESSORS_ONLN)"}