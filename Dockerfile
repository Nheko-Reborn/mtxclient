FROM ubuntu:14.04

ENV LIBSODIUM_VERSION=1.0.16
ENV SPDLOG_VERSION=1.1.0
ENV OLM_VERSION=2.2.2

ENV CMAKE_VERSION=3.12.1
ENV CMAKE_SHORT_VERSION=3.12

RUN \
    apt-get update -qq && \
    apt-get install -y apt-transport-https \
        software-properties-common \
        wget \
        ninja-build \
        curl \
        unzip git lcov \
        libssl-dev \
        openssl

RUN \
    add-apt-repository -y ppa:ubuntu-toolchain-r/test && \
    wget -O - https://apt.llvm.org/llvm-snapshot.gpg.key | apt-key add - && \
    apt-add-repository "deb https://apt.llvm.org/trusty/ llvm-toolchain-trusty-6.0 main" && \
    apt-get update -qq && \
    apt-get install -y \
        build-essential \
        clang++-6.0 \
        clang-6.0 \
        g++-8 \
        g++-5

RUN \
    sudo update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-8 10 && \
    sudo update-alternatives --install /usr/bin/g++ g++ /usr/bin/g++-8 10 && \
    sudo update-alternatives --set gcc "/usr/bin/gcc-8" && \
    sudo update-alternatives --set g++ "/usr/bin/g++-8"

RUN \
    mkdir -p /build/libsodium && cd /build/libsodium && \
    curl -L https://download.libsodium.org/libsodium/releases/libsodium-${LIBSODIUM_VERSION}.tar.gz -o libsodium-${LIBSODIUM_VERSION}.tar.gz && \
    tar xfz libsodium-${LIBSODIUM_VERSION}.tar.gz && cd /build/libsodium/libsodium-${LIBSODIUM_VERSION}/ && \
    ./configure && make && make check && make install

RUN \
    apt-get remove -y cmake && \
    wget https://cmake.org/files/v${CMAKE_SHORT_VERSION}/cmake-${CMAKE_VERSION}-Linux-x86_64.sh && \
    sudo sh cmake-${CMAKE_VERSION}-Linux-x86_64.sh  --skip-license  --prefix=/usr/local && \
    export PATH="/usr/local/bin:$PATH"

RUN \
    mkdir -p /build/spdlog && cd /build/spdlog && \
    curl -L https://github.com/gabime/spdlog/archive/v${SPDLOG_VERSION}.tar.gz -o spdlog-${SPDLOG_VERSION}.tar.gz && \
    tar xfz spdlog-${SPDLOG_VERSION}.tar.gz && cd /build/spdlog/spdlog-${SPDLOG_VERSION}/ && \
    cmake -H. -Bbuild -GNinja -DCMAKE_BUILD_TYPE=Release \
        -DSPDLOG_BUILD_EXAMPLES=0 \
        -DSPDLOG_BUILD_BENCH=0 \
        -DSPDLOG_BUILD_TESTING=0 && \
    cmake --build build --target install

RUN \
    mkdir -p /build/boost && cd /build/boost && \
    curl -L https://dl.bintray.com/boostorg/release/1.68.0/source/boost_1_68_0.tar.gz -o boost_1_68_0.tar.gz && \
    tar xfz boost_1_68_0.tar.gz && cd /build/boost/boost_1_68_0/ && \
    ./bootstrap.sh --with-libraries=random,thread,system,iostreams,atomic,chrono,date_time,regex && \
    ./b2 -d0 cxxstd=14 variant=release link=static threading=multi --layout=system && \
    ./b2 -d0 install

RUN \
    mkdir -p /build/gtest && cd /build/gtest && \
    curl -L https://github.com/google/googletest/archive/release-1.8.1.tar.gz -o release-1.8.1.tar.gz && \
    tar xfz release-1.8.1.tar.gz && cd /build/gtest/googletest-release-1.8.1/ && \
    cmake -H. -Bbuild -GNinja -DCMAKE_BUILD_TYPE=Release && \
    cmake --build build --target install

RUN \
    mkdir -p /build/olm && cd /build/olm && \
    git clone https://git.matrix.org/git/olm.git && \
    cd olm && mkdir -p cmake && \
    git checkout ${OLM_VERSION} && \
    curl -L https://raw.githubusercontent.com/mujx/mtxclient/master/deps/cmake/OlmCMakeLists.txt -o CMakeLists.txt && \
    curl -L https://raw.githubusercontent.com/mujx/mtxclient/master/deps/cmake/OlmConfig.cmake.in -o cmake/OlmConfig.cmake.in && \
    cmake -H. -Bbuild -GNinja -DCMAKE_BUILD_TYPE=Release && \
    cmake --build build --target install

RUN \
    apt-get install -y pkg-config && \
    curl -L https://github.com/nlohmann/json/releases/download/v3.2.0/json.hpp -o /usr/local/include/json.hpp

RUN \
    rm -rf /build/*

WORKDIR /build
