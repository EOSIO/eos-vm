function get_envs() {
    evars=""
    for e in $(printenv | awk -F= '{print $1}'); do
        evars+="-e $e "
    done
}


function docker-run() {
    get_envs
    execute eval docker run $ARGS $evars $FULL_TAG bash -c \"$@\"
}
