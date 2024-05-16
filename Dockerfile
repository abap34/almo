FROM alpine:latest

RUN apk add --no-cache g++ make

RUN apk add --no-cache bash

WORKDIR /app

COPY scripts /app/scripts
COPY src /app/src
COPY Makefile /app/Makefile

RUN make build

ENTRYPOINT [ "scripts/docker-entrypoint.sh" ]