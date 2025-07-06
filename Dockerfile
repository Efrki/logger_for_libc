FROM ubuntu:latest

RUN apt-get update && apt-get install -y \
    build-essential \
    gdb \
    man-db \
    Docker

WORKDIR /app

CMD ["/bin/bash"]
