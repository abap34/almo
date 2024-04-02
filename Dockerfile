FROM alpine:latest

RUN apk add --no-cache g++

WORKDIR /app

COPY . .

RUN sh build.sh

ENTRYPOINT [ "./docker-entrypoint.sh" ]
