# syntax=docker/dockerfile:1

FROM alpine:3.20 AS build

RUN apk add --no-cache build-base cmake git linux-headers

WORKDIR /src
COPY . .

RUN cmake -S . -B build \
        -DCMAKE_BUILD_TYPE=Release \
        -DPROCRELAY_STATIC=ON \
        -DPROCRELAY_BUILD_TESTS=OFF && \
    cmake --build build --target procrelay-server -j "$(nproc)" && \
    ! ldd build/sources/apps/server/procrelay-server && \
    strip build/sources/apps/server/procrelay-server

FROM scratch AS deploy

COPY --from=build /src/build/sources/apps/server/procrelay-server /procrelay-server

USER 65532

EXPOSE 8080

ENTRYPOINT ["/procrelay-server"]
