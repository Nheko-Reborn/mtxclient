FROM ubuntu:16.04

ENV LIBSODIUM_VERSION=1.0.16
ENV SPDLOG_VERSION=1.1.0
ENV OLM_VERSION=2.2.2
ENV NLOHMANN_VERSION=v3.2.0
ENV CMAKE_VERSION=3.15.5
ENV CMAKE_SHORT_VERSION=3.15

RUN \
    apt-get update -qq && \
    apt-get install -y --no-install-recommends apt-transport-https software-properties-common curl ninja-build && \
    # cmake
    curl https://cmake.org/files/v${CMAKE_SHORT_VERSION}/cmake-${CMAKE_VERSION}-Linux-x86_64.sh -o cmake-install.sh && \
    bash cmake-install.sh --skip-license --prefix=/usr/local && \
    export PATH="/usr/local/bin:$PATH" && \
    rm -f /*.sh && \
    # Toolchains
    add-apt-repository -y ppa:ubuntu-toolchain-r/test && \
    curl -L https://apt.llvm.org/llvm-snapshot.gpg.key | apt-key add - && \
    apt-add-repository "deb https://apt.llvm.org/xenial/ llvm-toolchain-xenial-6.0 main" && \
    apt-get update -qq && \
    apt-get install -y --no-install-recommends \
        ninja-build \
        pkg-config \
        curl \
        make \
        clang++-6.0 \
        clang-6.0 \
        g++-8 \
        g++-5 \
        unzip git lcov \
        libssl-dev \
        openssl && \
    sudo update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-8 10 && \
    sudo update-alternatives --install /usr/bin/g++ g++ /usr/bin/g++-8 10 && \
    sudo update-alternatives --install /usr/bin/gcov gcov /usr/bin/gcov-8 10 && \
    sudo update-alternatives --set gcc "/usr/bin/gcc-8" && \
    sudo update-alternatives --set g++ "/usr/bin/g++-8" && \
    sudo update-alternatives --set gcov "/usr/bin/gcov-8" && \
    # libsodium
    mkdir -p /build/libsodium && cd /build/libsodium && \
    curl -L https://download.libsodium.org/libsodium/releases/libsodium-${LIBSODIUM_VERSION}.tar.gz -o libsodium-${LIBSODIUM_VERSION}.tar.gz && \
    tar xfz libsodium-${LIBSODIUM_VERSION}.tar.gz && cd /build/libsodium/libsodium-${LIBSODIUM_VERSION}/ && \
    ./configure && make && make check && make install && \
    # spdlog
    mkdir -p /build/spdlog && cd /build/spdlog && \
    curl -L https://github.com/gabime/spdlog/archive/v${SPDLOG_VERSION}.tar.gz -o spdlog-${SPDLOG_VERSION}.tar.gz && \
    tar xfz spdlog-${SPDLOG_VERSION}.tar.gz && cd /build/spdlog/spdlog-${SPDLOG_VERSION}/ && \
    cmake -H. -Bbuild -GNinja -DCMAKE_BUILD_TYPE=Release \
        -DSPDLOG_BUILD_EXAMPLES=0 \
        -DSPDLOG_BUILD_BENCH=0 \
        -DSPDLOG_BUILD_TESTING=0 && \
    cmake --build build --target install && \
    # boost
    mkdir -p /build/boost && cd /build/boost && \
    curl -L https://dl.bintray.com/boostorg/release/1.70.0/source/boost_1_70_0.tar.gz -o boost_1_70_0.tar.gz && \
    tar xfz boost_1_70_0.tar.gz && cd /build/boost/boost_1_70_0/ && \
    ./bootstrap.sh --with-libraries=random,thread,system,iostreams,atomic,chrono,date_time,regex && \
    ./b2 -d0 cxxstd=17 variant=release link=static threading=multi --layout=system && \
    ./b2 -d0 install && \
    # Gtest
    mkdir -p /build/gtest && cd /build/gtest && \
    curl -L https://github.com/google/googletest/archive/release-1.8.1.tar.gz -o release-1.8.1.tar.gz && \
    tar xfz release-1.8.1.tar.gz && cd /build/gtest/googletest-release-1.8.1/ && \
    cmake -H. -Bbuild -GNinja -DCMAKE_BUILD_TYPE=Release && \
    cmake --build build --target install && \
    # libolm
    mkdir -p /build/olm && cd /build/olm && \
    git clone https://git.matrix.org/git/olm.git && \
    cd olm && mkdir -p cmake && \
    git checkout ${OLM_VERSION} && \
    curl -L https://raw.githubusercontent.com/Nheko-Reborn/mtxclient/master/deps/cmake/OlmCMakeLists.txt -o CMakeLists.txt && \
    mkdir -p cmake && \
    curl -L https://raw.githubusercontent.com/Nheko-Reborn/mtxclient/master/deps/cmake/OlmConfig.cmake.in -o cmake/OlmConfig.cmake.in && \
    cmake -H. -Bbuild -GNinja -DCMAKE_BUILD_TYPE=Release && \
    cmake --build build --target install && \
    # json.hpp
    mkdir /build/json && cd /build/json && \
    git clone --branch ${NLOHMANN_VERSION} --depth 1 https://github.com/nlohmann/json && \
    cd json && \
    cmake . && \
    make && \
    make install && \
    #curl -L https://github.com/nlohmann/json/releases/download/v3.2.0/json.hpp -o /usr/local/include/json.hpp && \
    rm -rf /build/* && \
    rm -rf /var/lib/apt/lists/* && \
    apt-get clean && \
    apt-get autoclean && \
    apt-get autoremove

WORKDIR /build
