# ==========================================
# STAGE 1: The Builder Environment
# ==========================================
FROM ubuntu:22.04 AS builder

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y \
    g++ \
    cmake \
    make \
    git \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app

COPY . .

RUN cmake -B build && make -j4 -C build

# ==========================================
# STAGE 2: The Production Environment
# ==========================================
FROM ubuntu:22.04

WORKDIR /app

COPY --from=builder /app/build/webserver .
COPY --from=builder /app/server.conf .
COPY --from=builder /app/public ./public

EXPOSE 8080
CMD ["./webserver"]