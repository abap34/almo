FROM alpine:latest

RUN apk add --no-cache g++

RUN apk add --no-cache bash

WORKDIR /app

COPY . .

RUN bash build.sh

ENTRYPOINT [ "./docker-entrypoint.sh" ]
