FROM alpine:3.7

ENV calib=/app/Files/Teddy/calib.txt
ENV ppm=/app/Files/Teddy/Frames/%04i.ppm
ENV pgm=/app/Files/Teddy/Frames/%04i.pgm

RUN apk update
RUN apk upgrade
RUN apk add make
RUN apk add cmake
RUN apk add --virtual build-dependencies
RUN apk add build-base
RUN apk add mesa-dev
RUN apk add freeglut-dev
RUN apk add mesa-dri-nouveau mesa-dri-swrast mesa-dri-vmwgfx


RUN rm -rf /var/cache/apk/*

COPY docker-entrypoint.sh /

COPY . /app/
WORKDIR /app

RUN mkdir build_linux
WORKDIR /app/build_linux

ENTRYPOINT /docker-entrypoint.sh $calib $ppm $pgm
