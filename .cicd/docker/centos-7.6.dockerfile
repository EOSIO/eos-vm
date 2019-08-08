FROM centos:7.6.1810
# install dependencies
RUN yum update -y && \
    yum install -y --enablerepo=extras centos-release-scl && \
    yum install -y --enablerepo=extras devtoolset-7 && \
    yum install -y --enablerepo=extras git sudo tar bzip2 make doxygen
# build cmake
RUN source /opt/rh/devtoolset-7/enable && \
    curl -LO https://cmake.org/files/v3.13/cmake-3.13.2.tar.gz && \
    tar -xzf cmake-3.13.2.tar.gz && \
    cd cmake-3.13.2 && \
    ./bootstrap --prefix=/usr/local && \
    make -j $(nproc) && \
    make install && \
    rm -f /cmake-3.13.2.tar.gz && rm -rf /cmake-3.13.2
# build clang
RUN source /opt/rh/devtoolset-7/enable && \
    git clone --single-branch --branch release_80 https://git.llvm.org/git/llvm.git clang8 && cd clang8 && git checkout 18e41dc && \
    cd tools && git clone --single-branch --branch release_80 https://git.llvm.org/git/lld.git && cd lld && git checkout d60a035 && \
    cd ../ && git clone --single-branch --branch release_80 https://git.llvm.org/git/polly.git && cd polly && git checkout 1bc06e5 && \
    cd ../ && git clone --single-branch --branch release_80 https://git.llvm.org/git/clang.git clang && cd clang && git checkout a03da8b && \
    cd tools && mkdir extra && cd extra && git clone --single-branch --branch release_80 https://git.llvm.org/git/clang-tools-extra.git && cd clang-tools-extra && git checkout 6b34834 && \
    cd /clang8/projects && git clone --single-branch --branch release_80 https://git.llvm.org/git/libcxx.git && cd libcxx && git checkout 1853712 && \
    cd ../ && git clone --single-branch --branch release_80 https://git.llvm.org/git/libcxxabi.git && cd libcxxabi && git checkout d7338a4 && \
    cd ../ && git clone --single-branch --branch release_80 https://git.llvm.org/git/libunwind.git && cd libunwind && git checkout 57f6739 && \
    cd ../ && git clone --single-branch --branch release_80 https://git.llvm.org/git/compiler-rt.git && cd compiler-rt && git checkout 5bc7979 && \
    mkdir /clang8/build && cd /clang8/build && \
    cmake -G 'Unix Makefiles' -DCMAKE_INSTALL_PREFIX='/usr/local' -DLLVM_BUILD_EXTERNAL_COMPILER_RT=ON -DLLVM_BUILD_LLVM_DYLIB=ON -DLLVM_ENABLE_LIBCXX=ON -DLLVM_ENABLE_RTTI=ON -DLLVM_INCLUDE_DOCS=OFF -DLLVM_OPTIMIZED_TABLEGEN=ON -DLLVM_TARGETS_TO_BUILD=X86 -DCMAKE_BUILD_TYPE=Release .. && \
    make -j $(nproc) && \
    make install && \
    rm -rf /clang8
# ccache
RUN curl -LO http://download-ib01.fedoraproject.org/pub/epel/7/x86_64/Packages/c/ccache-3.3.4-1.el7.x86_64.rpm && \
    yum install -y ccache-3.3.4-1.el7.x86_64.rpm