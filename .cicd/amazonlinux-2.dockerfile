FROM amazonlinux:2

ENV DCMAKE_TOOLCHAIN_FILE clang.make

# YUM dependencies.
RUN yum update -y \
  && yum install -y git sudo tar bzip2 make gcc gcc-c++ doxygen

# Build appropriate version of CMake.
RUN curl -LO https://cmake.org/files/v3.13/cmake-3.13.2.tar.gz \
  && tar -xzf cmake-3.13.2.tar.gz \
  && cd cmake-3.13.2 \
  && ./bootstrap --prefix=/usr/local \
  && make -j$(nproc) \
  && make install \
  && cd .. \
  && rm -f cmake-3.13.2.tar.gz

# Build appropriate version of Clang.
RUN mkdir -p /root/tmp && cd /root/tmp && git clone --single-branch --branch release_80 https://git.llvm.org/git/llvm.git clang8 \
  && cd clang8 && git checkout 18e41dc && cd tools && git clone --single-branch --branch release_80 https://git.llvm.org/git/lld.git \
  && cd lld && git checkout d60a035 && cd ../ && git clone --single-branch --branch release_80 https://git.llvm.org/git/polly.git \
  && cd polly && git checkout 1bc06e5 && cd ../ && git clone --single-branch --branch release_80 https://git.llvm.org/git/clang.git clang \
  && cd clang && git checkout a03da8b && cd tools && mkdir extra && cd extra && git clone --single-branch --branch release_80 https://git.llvm.org/git/clang-tools-extra.git \
  && cd clang-tools-extra && git checkout 6b34834 && cd .. && cd ../../../../projects && git clone --single-branch --branch release_80 https://git.llvm.org/git/libcxx.git \
  && cd libcxx && git checkout 1853712 && cd ../ && git clone --single-branch --branch release_80 https://git.llvm.org/git/libcxxabi.git && cd libcxxabi \
  && git checkout d7338a4 && cd ../ && git clone --single-branch --branch release_80 https://git.llvm.org/git/libunwind.git && cd libunwind && git checkout 57f6739 \
  && cd ../ && git clone --single-branch --branch release_80 https://git.llvm.org/git/compiler-rt.git && cd compiler-rt && git checkout 5bc7979 && cd ../ && cd /root/tmp/clang8 \
  && mkdir build && cd build && cmake -G 'Unix Makefiles' -DCMAKE_INSTALL_PREFIX='/usr/local' -DLLVM_BUILD_EXTERNAL_COMPILER_RT=ON -DLLVM_BUILD_LLVM_DYLIB=ON -DLLVM_ENABLE_LIBCXX=ON -DLLVM_ENABLE_RTTI=ON -DLLVM_INCLUDE_DOCS=OFF -DLLVM_OPTIMIZED_TABLEGEN=ON -DLLVM_TARGETS_TO_BUILD=all -DCMAKE_BUILD_TYPE=Release .. \
  && make -j$(nproc) && make install && cd / && rm -rf /root/tmp/clang8

# Build appropriate version of LLVM.
RUN git clone --depth 1 --single-branch --branch release_40 https://github.com/llvm-mirror/llvm.git llvm \
  && cd llvm \
  && mkdir build \
  && cd build \
  && cmake -G 'Unix Makefiles' -DLLVM_TARGETS_TO_BUILD=host -DLLVM_BUILD_TOOLS=false -DLLVM_ENABLE_RTTI=1 -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr/local .. \
  && make -j$(nproc) \
  && make install \
  && cd /
  
# CCACHE
RUN curl -LO http://download-ib01.fedoraproject.org/pub/epel/7/x86_64/Packages/c/ccache-3.3.4-1.el7.x86_64.rpm \
  && yum install -y ccache-3.3.4-1.el7.x86_64.rpm

# Setup clang file to use in cmake
RUN echo 'set(OPT_PATH @)' > $DCMAKE_TOOLCHAIN_FILE \
&& echo 'set(CMAKE_C_COMPILER_WORKS 1)' >> $DCMAKE_TOOLCHAIN_FILE \
&& echo 'set(CMAKE_CXX_COMPILER_WORKS 1)' >> $DCMAKE_TOOLCHAIN_FILE \
&& echo 'set(CMAKE_C_COMPILER /usr/local/bin/clang)' >> $DCMAKE_TOOLCHAIN_FILE \
&& echo 'set(CMAKE_CXX_COMPILER /usr/local/bin/clang++)' >> $DCMAKE_TOOLCHAIN_FILE \
&& echo 'set(CMAKE_CXX_STANDARD_INCLUDE_DIRECTORIES /usr/local/include/c++/v1 /usr/local/include /usr/include)' >> $DCMAKE_TOOLCHAIN_FILE \
&& echo 'set(CMAKE_CXX_FLAGS_INIT "-nostdinc++")' >> $DCMAKE_TOOLCHAIN_FILE \
&& echo 'set(CMAKE_EXE_LINKER_FLAGS_INIT "-stdlib=libc++ -nostdlib++")' >> $DCMAKE_TOOLCHAIN_FILE \
&& echo 'set(CMAKE_SHARED_LINKER_FLAGS_INIT "-stdlib=libc++ -nostdlib++")' >> $DCMAKE_TOOLCHAIN_FILE \
&& echo 'set(CMAKE_MODULE_LINKER_FLAGS_INIT "-stdlib=libc++ -nostdlib++")' >> $DCMAKE_TOOLCHAIN_FILE \
&& echo 'set(CMAKE_CXX_STANDARD_LIBRARIES "/usr/local/lib/libc++.a /usr/local/lib/libc++abi.a")' >> $DCMAKE_TOOLCHAIN_FILE

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
