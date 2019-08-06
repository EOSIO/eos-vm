function get-envs() {
    if [[ -f $BUILDKITE_ENV_FILE ]]; then
        evars=""
        while read -r var; do
            evars="$evars --env ${var%%=*}"
        done < "$BUILDKITE_ENV_FILE"
    fi
}

function docker-run() {
    get-envs
    execute eval docker run $ARGS $evars $FULL_TAG bash -c \"$@\"
}

function append-to-commands() {
    [[ ! -z $COMMANDS ]] && export COMMANDS="$COMMANDS && $@" || export COMMANDS="$@"
}