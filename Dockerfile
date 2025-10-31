FROM ubuntu:22.04

RUN apt-get update && apt-get install -y \
    gcc-arm-linux-gnueabihf \
    g++-arm-linux-gnueabihf \
    qemu-user-static \
    make \
    git \
    python3

WORKDIR /workspace
