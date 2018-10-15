# Author: David Blyth

ARG BASE=proio-bench:base
ARG BUILD_THREADS=5

FROM $BASE

# LHADPDF6
RUN git clone https://github.com/decibelcooper/LHAPDF.git \
&&  cd LHAPDF \
&&  CC=gcc CXX=g++ ./configure --prefix=/usr/local --disable-python \
&&  make -j $BUILD_THREADS \
&&  sudo make install \
&&  wget -q https://lhapdf.hepforge.org/downloads?f=pdfsets/6.2.1/CT14lo.tar.gz -O CT14lo.tar.gz \
&&  sudo tar -xzf CT14lo.tar.gz -C /usr/local/share/LHAPDF \
&&  cd ../ \
&&  rm -rf LHAPDF

# Pythia8
RUN git clone https://github.com/decibelcooper/pythia8.git \
&&  cd pythia8 \
&&  ./configure --prefix=/usr/local --enable-shared --with-lhapdf6 \
&&  make -j $BUILD_THREADS \
&&  sudo make install \
&&  cd ../ \
&&  rm -rf pythia8

# Generate data
ADD src/generate.cc src/
ADD cards cards

RUN g++ -o bin/generate \
        -O3 \
        src/generate.cc \
        `pythia8-config --cpp-flags --libs` \
        `root-config --cflags --libs` \
        -lprotobuf -lproio -lproio.pb

RUN mkdir gev35ep_pythia8_gev1q2 \
&&  cd gev35ep_pythia8_gev1q2 \
&&  generate ../cards/gev35ep_pythia8_gev1q2 200000

RUN mkdir tev13_pythia8_qcdjet_pt50 \
&&  cd tev13_pythia8_qcdjet_pt50 \
&&  generate ../cards/tev13_pythia8_qcdjet_pt50 4000

# Install tools provided by package manager
RUN sudo apt-get install --assume-yes \
        google-perftools \
        libgoogle-perftools-dev \
&&  sudo apt-get clean

# Reencode tests
ADD src/reencode_root.cc src/

RUN g++ -o bin/reencode_root \
        -O3 \
        src/reencode_root.cc \
        `root-config --cflags --libs`

ADD src/reencode_proio.cc src/

RUN g++ --std=c++11 -o bin/reencode_proio \
        -O3 \
        src/reencode_proio.cc \
        -lproio -lproio.pb -lprotobuf

# Decode tests
ADD src/decode_root.cc src/

RUN g++ --std=c++11 -o bin/decode_root \
        -O3 \
        src/decode_root.cc \
        `root-config --cflags --libs`

ADD src/decode_proio.cc src/

RUN g++ --std=c++11 -o bin/decode_proio \
        -O3 \
        src/decode_proio.cc \
        -lproio -lproio.pb -lprotobuf

# Set default behavior
ADD bin bin

CMD bash -v benchmark
