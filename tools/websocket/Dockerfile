# Build context: tools/websocket. Mount the configuration at /config/config.json.
FROM rust:1-bookworm AS build
WORKDIR /src
COPY Cargo.toml Cargo.lock ./
COPY src ./src
RUN cargo build --release --locked --bins

FROM debian:bookworm-slim
RUN useradd --system --no-create-home --shell /usr/sbin/nologin gateway
COPY --from=build /src/target/release/gateway /src/target/release/client-fixture /usr/local/bin/
USER gateway
EXPOSE 8080
CMD ["gateway", "/config/config.json"]
