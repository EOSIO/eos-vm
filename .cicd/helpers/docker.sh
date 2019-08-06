function get_envs() {
    if [[ -f $BUILDKITE_ENV_FILE ]]; then
        evars=""
        while read -r var; do
            evars="$evars --env ${var%%=*}"
            echo $evars
        done < "$BUILDKITE_ENV_FILE"
    fi
}


function docker-run() {
    get_envs
    execute eval docker run $ARGS $evars $FULL_TAG bash -c \"$@\"
}
