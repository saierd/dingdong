FROM debian:buster

RUN apt-get update \
 && apt-get -y install --no-install-recommends \
    cmake \
    dpkg-dev \
    file \
    g++-arm-linux-gnueabihf \
    ninja-build \
 && rm -rf /var/lib/apt/lists/*

# Install ARM library dependencies.
RUN dpkg --add-architecture armhf \
 && apt-get update \
 && apt-get -y install --no-install-recommends \
    libblkid1:armhf \
    libglib2.0-dev:armhf \
    libgstreamer1.0-dev:armhf \
    libgtkmm-3.0-dev:armhf \
    libmount-dev:armhf \
    libpulse-dev:armhf \
    libssl-dev:armhf \
    libx11-dev:armhf \
    libxfixes-dev:armhf \
    uuid-dev:armhf \
    x11proto-dev \
&& rm -rf /var/lib/apt/lists/*

ENV CC=arm-linux-gnueabihf-gcc \
    CXX=arm-linux-gnueabihf-g++ \
    CMAKE_C_COMPILER=arm-linux-gnueabihf-gcc \
    CMAKE_CXX_COMPILER=arm-linux-gnueabihf-g++ \
    STRIP=arm-linux-gnueabihf-strip \
    RANLIB=arm-linux-gnueabihf-ranlib \
    AS=arm-linux-gnueabihf-as \
    AR=arm-linux-gnueabihf-ar \
    LD=arm-linux-gnueabihf-ld

VOLUME /workspace
WORKDIR /workspace
