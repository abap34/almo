FROM ubuntu:22.04

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y \
    build-essential \
    meson \
    ninja-build \
    gcc \
    g++ \
    lcov \
    gcovr \
    git \
    python3-pip \
    python3-setuptools \
    && apt-get clean
