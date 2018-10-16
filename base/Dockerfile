# Author: David Blyth

FROM rootproject/root-ubuntu16:snapshot

ARG BUILD_THREADS=5

WORKDIR /home/builder

CMD /bin/bash

# Protobuf
ENV LZ4_VERSION=v1.8.3

RUN git clone https://github.com/lz4/lz4.git \
&&  cd lz4 \
&&  if [ "$LZ4_VERSION" != "latest" ]; then git checkout tags/$LZ4_VERSION; fi \
&&  make -j $BUILD_THREADS \
&&  sudo make install \
&&  cd ../ \
&&  rm -rf lz4

ENV PROTOBUF_VERSION=v3.6.1

RUN git clone https://github.com/google/protobuf.git \
&&  cd protobuf \
&&  if [ "$PROTOBUF_VERSION" != "latest" ]; then git checkout tags/$PROTOBUF_VERSION; fi \
&&  bash autogen.sh \
&&  ./configure \
&&  make -j $BUILD_THREADS \
&&  sudo make install \
&&  cd ../ \
&&  rm -rf protobuf

ENV LD_LIBRARY_PATH=/usr/local/lib:${LD_LIBRARY_PATH}

# go-proio
RUN sudo apt-get update \
&&  sudo apt-get install --assume-yes \
        wget \
&&  sudo apt-get clean

ENV GOPATH=${HOME} \
    GOROOT=${HOME}/go

RUN wget -q https://dl.google.com/go/go1.11.1.linux-amd64.tar.gz -O go.tar.gz \
&&  tar -xzf go.tar.gz \
&&  rm go.tar.gz

ENV GO_PROIO_CHECKOUT=1acbe5e272eb12c7f1d84873a9aa466b3ac27b16 \
    PATH=$GOPATH/bin:$GOROOT/bin:${PATH}

RUN git clone https://github.com/proio-org/go-proio.git \
&&  cd go-proio \
&&  git checkout $GO_PROIO_CHECKOUT \
&&  go get ./... \
&&  cd .. \
&&  sudo rm -rf go-proio $GOPATH/src $GOPATH/pkg

# py-proio
RUN sudo apt-get install --assume-yes \
        python-pip \
&&  sudo apt-get clean

ENV PY_PROIO_VERSION=0.12.63

RUN sudo pip install proio==$PY_PROIO_VERSION

# cpp-proio
ENV CPP_PROIO_CHECKOUT=f0b989d55da5d8aba8ae35e096e729fdf0d21814

RUN git clone https://github.com/proio-org/cpp-proio.git \
&&  mkdir cpp-proio/build \
&&  cd cpp-proio/build \
&&  git checkout $CPP_PROIO_CHECKOUT \
&&  git submodule init \
&&  git submodule update \
&&  cmake ../ \
&&  make -j $BUILD_THREADS \
&&  sudo make install \
&&  cd ../../ \
&&  rm -rf cpp-proio
