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
ADD generate.cc ./
ADD cards cards

RUN g++ -o generate \
        generate.cc \
        `pythia8-config --cpp-flags --libs` \
        `root-config --cflags --libs` \
        -lprotobuf -lproio -lproio.pb

RUN mkdir gev35ep_pythia8_gev1q2 \
&&  cd gev35ep_pythia8_gev1q2 \
&&  ../generate ../cards/gev35ep_pythia8_gev1q2 50000

RUN mkdir tev13_pythia8_qcdjet_pt50 \
&&  cd tev13_pythia8_qcdjet_pt50 \
&&  ../generate ../cards/tev13_pythia8_qcdjet_pt50 1000

# Reencode data
ADD reencode_root.cc ./

RUN g++ -o reencode_root \
        reencode_root.cc \
        `root-config --cflags --libs`

RUN cd gev35ep_pythia8_gev1q2 \
&&  ../reencode_root -a lzma -l 5 int_packed_particles_uncomp.root int_packed_particles_lzma.root

ADD reencode_proio.cc ./

RUN g++ --std=c++11 -o reencode_proio \
        reencode_proio.cc \
        -lproio -lproio.pb -lprotobuf

RUN cd gev35ep_pythia8_gev1q2 \
&&  ../reencode_proio -a lz4 varint_packed_particles_uncomp.proio varint_packed_particles_lz4.proio
