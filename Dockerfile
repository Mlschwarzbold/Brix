FROM debian:bookworm-20250908-slim

# Update package lists and install CMake and build essentials
RUN apt-get update && \
    apt-get install -y --no-install-recommends \
    cmake \
    iproute2 \
    build-essential && \
    rm -rf /var/lib/apt/lists/*

WORKDIR /brix

COPY . /brix

RUN ./urubu.sh clean

ENTRYPOINT ["sh", "/brix/urubu.sh"]