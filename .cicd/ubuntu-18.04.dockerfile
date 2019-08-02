FROM ubuntu:18.04
RUN apt-get update && \
    apt-get install -y git cmake clang-8 libc++-8* libc++abi-8* ccache
CMD bash -c "cd /workdir && \
    ccache -s && \
    echo '+++ :git: Updating Submodules' && \
    git submodule update --init --recursive && \
    echo '+++ :hammer: Building eos-vm' && \
    mkdir -p build && cd build && \
    cmake -DCMAKE_TOOLCHAIN_FILE=/workdir/.cicd/clang.make -DENABLE_TESTS=ON .. && \
    make -j$(getconf _NPROCESSORS_ONLN) && \
    echo '+++ :white_check_mark: Done!'"