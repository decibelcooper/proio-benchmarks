# Author: David Blyth

FROM dbcooper/arch:2018-09-05

ARG BUILD_THREADS=5

# Set up basic environment
RUN pacman -S --noconfirm \
        sed \
        sudo \
&&  rm -rf /var/cache/pacman/pkg

RUN useradd -m -G wheel user \
&&  sed -i.bak 's/\s*#\s*\(%wheel\s*ALL=(ALL)\s*NOPASSWD:\s*ALL\)/\1/' /etc/sudoers

USER user
WORKDIR /home/user

CMD /bin/bash

# ROOT
ENV ROOT_RELEASE=v6-13-08 \
    CC=/usr/bin/clang \
    CXX=/usr/bin/clang++

RUN sudo pacman -S --noconfirm \
        binutils \
        clang \
        cmake \
        git \
        grep \
        gzip \
        libx11 \
        libxft \
        libxpm \
        make \
        python2 \
&&  sudo rm -rf /var/cache/pacman/pkg

RUN git clone http://root.cern.ch/git/root.git \
&&  cd root \
&&  git checkout tags/$ROOT_RELEASE \
&&  cd .. \
&&  mkdir build \
&&  cd build \
&&  cmake \
        -DCMAKE_INSTALL_PREFIX=/usr/local \
        -DCMAKE_BUILD_TYPE=Release \
        ../root \
&&  make -j $BUILD_THREADS \
&&  sudo make install \
&&  cd .. \
&&  rm -rf build root

ENV PATH=/usr/local/bin:${PATH} \
    LD_LIBRARY_PATH=/usr/local/lib \
    PYTHONPATH=/usr/local/lib \
    ROOTSYS=/usr/local

# Protobuf
RUN sudo pacman -S --noconfirm \
        protobuf \
        protobuf-c \
        python-protobuf \
        python2-protobuf \
&&  sudo rm -rf /var/cache/pacman/pkg

# go-proio
ENV GOPATH=/opt/Go
ENV GO_PROIO_CHECKOUT=v0.3.0 \
    PATH=$GOPATH/bin:${PATH}

RUN sudo pacman -S --noconfirm \
        go \
&&  sudo rm -rf /var/cache/pacman/pkg

RUN git clone https://github.com/proio-org/go-proio.git \
&&  cd go-proio \
&&  git checkout $GO_PROIO_CHECKOUT \
&&  go get ./... \
&&  cd .. \
&&  sudo rm -rf go-proio $GOPATH/src $GOPATH/pkg

# py-proio
ENV PY_PROIO_VERSION=0.12.52

RUN sudo pacman -S --noconfirm \
        python-pip \
        python2-pip \
&&  sudo rm -rf /var/cache/pacman/pkg

RUN sudo pip install proio==$PY_PROIO_VERSION \
&&  sudo pip2 install proio==$PY_PROIO_VERSION

# cpp-proio
ENV CPP_PROIO_CHECKOUT=a0a480a77a8dcb664406294204f8dbd782afcc19

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

# LHADPDF6
RUN sudo pacman -S --noconfirm \
        autoconf \
        automake \
        diffutils \
        file \
        wget \
&&  sudo rm -rf /var/cache/pacman/pkg

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
RUN sudo pacman -S --noconfirm \
        rsync \
&&  sudo rm -rf /var/cache/pacman/pkg

RUN git clone https://github.com/decibelcooper/pythia8.git \
&&  cd pythia8 \
&&  ./configure --prefix=/usr/local --enable-shared --with-lhapdf6 \
&&  make -j $BUILD_THREADS \
&&  sudo make install \
&&  cd ../ \
&&  rm -rf pythia8

# Run Pythia8
ADD pythia8bench pythia8bench

RUN sudo chmod o+w pythia8bench \
&&  cd pythia8bench \
&&  clang++ -o pythia8bench \
        main.cc `pythia8-config --cpp-flags --libs` -lprotobuf -lproio -lproio.pb

RUN cd pythia8bench \
&&  mkdir gev35ep_pythia8_gev1q2 \
&&  cd gev35ep_pythia8_gev1q2 \
&&  ../pythia8bench ../cards/gev35ep_pythia8_gev1q2 50000

RUN cd pythia8bench \
&&  mkdir tev13_pythia8_qcdjet_pt50 \
&&  cd tev13_pythia8_qcdjet_pt50 \
&&  ../pythia8bench ../cards/tev13_pythia8_qcdjet_pt50 1000

# Convenience tools
RUN sudo pacman -S --noconfirm \
        procps-ng \
        wget \
&&  sudo rm -rf /var/cache/pacman/pkg
