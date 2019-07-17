FROM ubuntu:18.04

ENV DCMAKE_TOOLCHAIN_FILE clang.make

RUN apt update && apt install -y git cmake clang-8 libc++-8* libc++abi-8*

# Setup clang file to use in cmake
RUN echo 'set(OPT_PATH @)' > $DCMAKE_TOOLCHAIN_FILE \
&& echo 'set(CMAKE_C_COMPILER_WORKS 1)' >> $DCMAKE_TOOLCHAIN_FILE \
&& echo 'set(CMAKE_CXX_COMPILER_WORKS 1)' >> $DCMAKE_TOOLCHAIN_FILE \
&& echo 'set(CMAKE_C_COMPILER /usr/bin/clang-8)' >> $DCMAKE_TOOLCHAIN_FILE \
&& echo 'set(CMAKE_CXX_COMPILER /usr/bin/clang++-8)' >> $DCMAKE_TOOLCHAIN_FILE \
&& echo 'set(CMAKE_CXX_STANDARD_INCLUDE_DIRECTORIES /usr/lib/llvm-8/include/c++/v1 /usr/local/include /usr/include)' >> $DCMAKE_TOOLCHAIN_FILE \
&& echo 'set(CMAKE_CXX_FLAGS_INIT "-nostdinc++")' >> $DCMAKE_TOOLCHAIN_FILE \
&& echo 'set(CMAKE_EXE_LINKER_FLAGS_INIT "-stdlib=libc++ -nostdlib++")' >> $DCMAKE_TOOLCHAIN_FILE \
&& echo 'set(CMAKE_SHARED_LINKER_FLAGS_INIT "-stdlib=libc++ -nostdlib++")' >> $DCMAKE_TOOLCHAIN_FILE \
&& echo 'set(CMAKE_MODULE_LINKER_FLAGS_INIT "-stdlib=libc++ -nostdlib++")' >> $DCMAKE_TOOLCHAIN_FILE \
&& echo 'set(CMAKE_CXX_STANDARD_LIBRARIES "/usr/lib/llvm-8/lib/libc++.a /usr/lib/llvm-8/lib/libc++abi.a")' >> $DCMAKE_TOOLCHAIN_FILE

CMD bash -c "cd /workdir && \
echo '=== Updating Submodules' && \
git submodule update --init --recursive && \
echo '=== BUILDING $PROJECT_NAME' && \
mkdir -p build && cd build && \
mv /$DCMAKE_TOOLCHAIN_FILE . && \
cmake -DCMAKE_TOOLCHAIN_FILE=$DCMAKE_TOOLCHAIN_FILE -DENABLE_TESTS=ON .. && \
make -j$(getconf _NPROCESSORS_ONLN) && \
ctest -j$(getconf _NPROCESSORS_ONLN) -V -T Test && \
echo '=== COMPLETE'"