FROM centos:7.6.1810
ENV DCMAKE_TOOLCHAIN_FILE clang.make
# install dependencies
RUN yum update -y && yum install -y git sudo tar bzip2 make gcc gcc-c++ doxygen centos-release-scl && yum install -y devtoolset-8-gcc
# configure terminal to use new gcc included with dev tools
RUN echo '' >> ~/.bashrc && echo '### gcc ###' >> ~/.bashrc && echo 'export PATH="/opt/rh/devtoolset-8/root/usr/bin:$PATH"' >> ~/.bashrc
# build cmake
RUN curl -LO https://cmake.org/files/v3.13/cmake-3.13.2.tar.gz && \
    tar -xzf cmake-3.13.2.tar.gz && \
    cd cmake-3.13.2 && \
    ./bootstrap --prefix=/usr/local && \
    make -j$(nproc) && \
    make install && \
    cd .. && \
    rm -f cmake-3.13.2.tar.gz
# build clang
COPY ./build-clang.sh /
RUN /build-clang.sh
# build llvm
RUN git clone --depth 1 --single-branch --branch release_40 https://github.com/llvm-mirror/llvm.git llvm && \
    cd llvm && mkdir build && cd build && \
    cmake -G 'Unix Makefiles' -DLLVM_TARGETS_TO_BUILD=host -DLLVM_BUILD_TOOLS=false -DLLVM_ENABLE_RTTI=1 -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr/local .. && \
    make -j$(nproc) && \
    make install
# ccache
RUN curl -LO http://download-ib01.fedoraproject.org/pub/epel/7/x86_64/Packages/c/ccache-3.3.4-1.el7.x86_64.rpm && \
    yum install -y ccache-3.3.4-1.el7.x86_64.rpm
# configure cmake
RUN echo 'set(OPT_PATH @)' > $DCMAKE_TOOLCHAIN_FILE && \
    echo 'set(CMAKE_C_COMPILER_WORKS 1)' >> $DCMAKE_TOOLCHAIN_FILE && \
    echo 'set(CMAKE_CXX_COMPILER_WORKS 1)' >> $DCMAKE_TOOLCHAIN_FILE && \
    echo 'set(CMAKE_C_COMPILER /usr/local/bin/clang)' >> $DCMAKE_TOOLCHAIN_FILE && \
    echo 'set(CMAKE_CXX_COMPILER /usr/local/bin/clang++)' >> $DCMAKE_TOOLCHAIN_FILE && \
    echo 'set(CMAKE_CXX_STANDARD_INCLUDE_DIRECTORIES /usr/local/include/c++/v1 /usr/local/include /usr/include)' >> $DCMAKE_TOOLCHAIN_FILE && \
    echo 'set(CMAKE_CXX_FLAGS_INIT "-nostdinc++")' >> $DCMAKE_TOOLCHAIN_FILE && \
    echo 'set(CMAKE_EXE_LINKER_FLAGS_INIT "-stdlib=libc++ -nostdlib++")' >> $DCMAKE_TOOLCHAIN_FILE && \
    echo 'set(CMAKE_SHARED_LINKER_FLAGS_INIT "-stdlib=libc++ -nostdlib++")' >> $DCMAKE_TOOLCHAIN_FILE && \
    echo 'set(CMAKE_MODULE_LINKER_FLAGS_INIT "-stdlib=libc++ -nostdlib++")' >> $DCMAKE_TOOLCHAIN_FILE && \
    echo 'set(CMAKE_CXX_STANDARD_LIBRARIES "/usr/local/lib/libc++.a /usr/local/lib/libc++abi.a")' >> $DCMAKE_TOOLCHAIN_FILE
# instructions
CMD bash -c "cd /workdir && \
    ccache -s && \
    echo '+++ :git: Updating Submodules' && \
    git submodule update --init --recursive && \
    echo '+++ :hammer: Building eos-vm' && \
    mkdir -p build && cd build && \
    mv /$DCMAKE_TOOLCHAIN_FILE . && \
    cmake -DCMAKE_TOOLCHAIN_FILE=$DCMAKE_TOOLCHAIN_FILE -DENABLE_TESTS=ON .. && \
    make -j$(getconf _NPROCESSORS_ONLN) && \
    echo '+++ :white_check_mark: Done!'"
