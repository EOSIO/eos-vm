export IMAGE_TAG=${IMAGE_TAG:-'ubuntu-18.04'}

function execute()
{
  $VERBOSE && echo "--- Executing: $@"
  $DRYRUN || "$@"
}

function determine-hash()
{
    # Determine the sha1 hash of all dockerfiles in the .cicd directory.
    [[ -z $1 ]] && echo "Please provide the files to be hashed (wildcards supported)" && exit 1
    echo "Obtaining Hash of files from $1..."
    # Collect all files, hash them, then hash those.
    HASHES=()
    for FILE in $(find $1 -type f); do
        HASH=$(sha1sum $FILE | sha1sum | awk '{ print $1 }')
        HASHES=($HASH "${HASHES[*]}")
        echo "$FILE - $HASH"
    done
    export DETERMINED_HASH=$(echo ${HASHES[*]} | sha1sum | awk '{ print $1 }')
    export HASHED_IMAGE_TAG="${IMAGE_TAG}-${DETERMINED_HASH}"
}

determine-hash ".cicd/${IMAGE_TAG}.dockerfile"
export FULL_TAG="eosio/producer:eos-vm-$HASHED_IMAGE_TAG"