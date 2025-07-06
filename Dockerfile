FROM ubuntu:latest

RUN apt-get update && apt-get install -y \
    build-essential \
    gdb \
    man-db \
    Docker \
    clang-format \
    clangd \
    cmake \
    make

WORKDIR /app

CMD ["/bin/bash"]
