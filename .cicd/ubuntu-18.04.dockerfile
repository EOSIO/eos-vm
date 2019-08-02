FROM ubuntu:18.04
# install dependencies
RUN apt-get update && \
    apt-get install -y git cmake clang-8 libc++-8* libc++abi-8* ccache
# container entrypoint
CMD /workdir/.cicd/entrypoint.sh