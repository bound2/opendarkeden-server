# syntax=docker/dockerfile:1
#
# Build the DarkEden servers from source and package them into a runnable image.
#
#   docker build -t darkeden:local .
#
# docker/docker-compose.yml uses this Dockerfile, so `docker compose up` builds
# everything from this repository - no pre-built image is pulled from a registry.

# ----------------------------------------------------------------------------
# Stage 1: build - same toolchain as Dockerfile.dev, plus cmake
# ----------------------------------------------------------------------------
FROM ubuntu:20.04 AS builder

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y --no-install-recommends \
    build-essential \
    cmake \
    pkg-config \
    libxerces-c-dev \
    libmysqlclient-dev \
    liblua5.1-dev \
    zlib1g-dev \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /home/darkeden/vs

# Only the sources are needed to build; data/ and conf/ go into the final image.
COPY CMakeLists.txt ./
COPY src ./src

# Debug by default, like `make` and the rest of the project: a Release build
# defines NDEBUG, which turns Assert(expr) into ((void)0) - and ~165 call sites
# rely on the side effect inside Assert(), e.g.
#   Assert(pTree->GetAttribute("class", iClass));
# so a Release gameserver dies while loading the quest XML files.
ARG BUILD_TYPE=Debug
# Number of parallel compile jobs; defaults to all available cores.
ARG BUILD_JOBS=

RUN cmake -B build -DCMAKE_BUILD_TYPE=${BUILD_TYPE} \
    && cmake --build build -j "${BUILD_JOBS:-$(nproc)}"

# ----------------------------------------------------------------------------
# Stage 2: runtime - only the shared libraries the servers link against
# ----------------------------------------------------------------------------
FROM ubuntu:20.04

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y --no-install-recommends \
    libxerces-c3.2 \
    libmysqlclient21 \
    liblua5.1-0 \
    zlib1g \
    psmisc \
    && rm -rf /var/lib/apt/lists/*

# CMAKE_RUNTIME_OUTPUT_DIRECTORY puts loginserver/sharedserver/gameserver here.
COPY --from=builder /home/darkeden/vs/bin/ /home/darkeden/vs/bin/
COPY data /home/darkeden/vs/data/
COPY docker/conf /home/darkeden/vs/conf/
COPY docker/start.sh /home/darkeden/vs/bin/start.sh

# A Windows checkout can carry CRLF endings, which both the configuration
# parser (Properties::load) and /bin/bash choke on - normalize them here so the
# image builds correctly regardless of how the repository was cloned.
RUN sed -i 's/\r$//' /home/darkeden/vs/conf/*.conf /home/darkeden/vs/bin/start.sh \
    && chmod +x /home/darkeden/vs/bin/start.sh

WORKDIR /home/darkeden/vs/bin

# loginserver TCP, gameserver TCP, gameserver UDP
EXPOSE 9999 9998 9997/udp

CMD ["./start.sh"]
