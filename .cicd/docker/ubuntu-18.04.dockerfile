FROM ubuntu:18.04
# install dependencies
RUN apt-get update && \
    apt-get install -y git clang-8 libc++-8* libc++abi-8* ccache curl build-essential

RUN curl -LO https://cmake.org/files/v3.13/cmake-3.13.2.tar.gz && \
    tar -xzf cmake-3.13.2.tar.gz && \
    cd cmake-3.13.2 && \
    ./bootstrap --prefix=/usr/local && \
    make -j $(nproc) && \
    make install && \
    rm -f /cmake-3.13.2.tar.gz && rm -rf /cmake-3.13.2 && \
    apt autoremove -y build-essential

# container entrypoint
CMD /workdir/.cicd/docker/entrypoint.sh